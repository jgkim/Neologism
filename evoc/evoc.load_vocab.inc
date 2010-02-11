<?php

/**
 * Loads an RDF/XML file from an HTTP URI and parses it. Handles content negotiation
 * and various errors.
 *
 * @param $uri
 * @return Model
 * @throws Exception On network or parse error
 */
function evoc_read_RDF_from_URI($uri) {
    $uri = trim($uri);
    // check if it's an HTTP URI
    if (!preg_match('!^http://!', $uri)) {
        throw new Exception('Can only read from http:// URIs');
    }
    // create a custom context that sends an Accept header that asks for RDF/XML
    $context = stream_context_create(array('http' => array(
            'header' => 'Accept: application/rdf+xml, text/rdf+xml, text/xml;q=0.1, application/xml;q=0.1, text/plain;q=0.1',
    )));
    // read file contents
    set_error_handler('_evoc_read_RDF_from_URI__error_handler', E_WARNING);
    try {
        $content = file_get_contents($uri, FILE_BINARY, $context);
    } catch (Exception $ex) {
        $error = $ex->getMessage();
        $offset = strrpos($error, ': ');
        if ($offset) {
            $error = substr($error, $offset + 2);
        }
    }
    restore_error_handler();
    if (empty($content)) {
        // reading has failed.
        if (empty($http_response_header)) {
            // failure is NOT an HTTP error reported by the destination server,
            // so we will use PHP's warning message
            if ($error) {
                throw new Exception('Failed to read from URI: ' . $error);
            }
            throw new Exception('Failed to read from URI');
        }
        // First http_response_header should be: 'HTTP/1.1 404 Not Found' or similar.
        // Try to get only the '404 Not Found' part.
        if (preg_match('/^[^ ]* (.*)/', $http_response_header[0], $match)) {
            $http_error = $match[1];
        } else {
            $http_error = $http_response_header[0];
        }
        throw new Exception('Failed to read from URI: ' . $http_error);
    }
    // Remove Byte Order Mark if present
    if (strlen($content) >= 3 && ord($content[0]) == 0xEF && ord($content[1]) == 0xBB && ord($content[2]) == 0xBF) {
        $content = substr($content, 3);
    }

    // find media type by looking for a Content-Type header; default to application/rdf+xml
    $media_type = null;
    foreach (@$http_response_header as $header) {
        $header = trim(strtolower($header));
        if (preg_match('!^HTTP\/!i', $header)) {
            // In case of redirects, headers of subsequent redirects are simply appended to the array.
            // We are only interested in the last one, so reset.
            $media_type = null;
        }
        if (preg_match('/^content-type *: *([a-z0-9\/+-]+)/', $header, $match)) {
            $media_type = $match[1];
        }
    }
    $extension = null;
    if (preg_match('/\.([a-zA-Z0-9])$/', $uri, $match)) {
        $extension = strtolower($match[1]);
    }

    // Sanity checks: Did we actually load some RDF/XML?
    if ($media_type == 'application/rdf+xml' || $media_type == 'text/rdf' || $media_type == 'application/rdf') {
        // Media type says RDF/XML, so let's try it
    } else if ($extension == 'rdf' || $extension == 'rdfx' || $extension == 'owl') {
        // Extension looks like RDF/XML, so let's try it
    } else if (preg_match('!<rdf:RDF|http://www\.w3\.org/1999/02/22-rdf-syntax-ns#!', substr($content, 1000))) {
        // Content smells like RDF/XML, so let's try it
    } else if ($media_type == 'text/html' || $media_type == 'application/xhtml+xml') {
        // Bad! This is HTML, not RDF/XML
        throw new Exception('Server returned an HTML media type, but we can only process RDF/XML');
    } else if (preg_match('/n3|turtle|ttl/', $media_type)) {
        // Bad! This is N3 or Turtle, not RDF/XML
        throw new Exception('Server returned a Turtle media type, but we can only process RDF/XML');
    } else if (preg_match('/^\s*@prefix/', $content)) {
        // Bad! This starts like a Turtle or N3 file
        throw new Exception('This appears to be a Turtle document, but we can only process RDF/XML');
    } else if (preg_match('/^x?html?$/', $media_type)) {
        // Bad! This looks like HTML, not RDF/XML
        throw new Exception('This appears to be an HTML document, but we can only process RDF/XML');
    } else if ($media_type == 'text/plain' || $media_type == 'application/octet-stream') {
        // Generic/default media types, might be OK
    } else if ($media_type) {
        throw new Exception("Can only process RDF/XML; reported media type was $media_type");
    }
    if (substr(ltrim($content), 0, 1) != '<') {
        throw new Exception("Parse error: File does not start with '<'");
    }

    // Create a RAP model and parse the content
    // @todo TODO Port this RAP-specific code to ARC2
    include_once(RDFAPI_INCLUDE_DIR . "RdfAPI.php");
    include_once(RDFAPI_INCLUDE_DIR . "syntax/RdfParser.php");
    $model = ModelFactory::getDefaultModel();
    $parser = new RdfParser();
    set_error_handler('_evoc_read_RDF_from_URI__error_handler', E_USER_ERROR);
    try {
        $parser->generateModel($content, $uri, $model);
        restore_error_handler();
        return $model;
    } catch (Exception $ex) {
        restore_error_handler();
        throw $ex;
    }
}

function _evoc_read_RDF_from_URI__error_handler($errno, $errstr) {
    throw new Exception($errstr);
}


/**
 * Queries a RAP Model for classes and properties, and builds
 * an associative array describing the vocabulary and any 
 * classes and properties found.
 *
 * @param object $model A RAP model containing an RDFS vocabulary or OWL ontology
 * @param string $ns_prefix Namespace prefix for the vocabulary
 * @param string $ns_uri Only terms in this namespace will be considered
 * @return array Array describing the vocabulary, its classes and properties.
 */
function evoc_get_vocabulary_description(&$model, $ns_prefix, $ns_uri) {

  _evoc_prepare_import_namespaces($ns_uri, $ns_prefix, $model);

  $vocabulary = array(
      'prefix' => $ns_prefix,
      'uri' => $ns_uri,
      'title' => _evoc_query_for_title($model, $ns_uri),
      'description' => _evoc_query_for_description($model, $ns_uri),
      'body' => null,   // string with detailed HTML description
      'classes' => array(),
      'properties' => array(),
      'ignored_ns' => array(),
      'undefined_ns' => array(),
  );

  // Add description of each class to $vocabulary['classes']
  $class_uris = _evoc_query_for_class_uris($model, $ns_uri);
  $ignored_uris = array();
  foreach ($class_uris as $uri) {
    if (substr($uri, 0, strlen($ns_uri)) != $ns_uri) {
      $ignored_uris[] = $uri;
      continue;
    }
    $vocabulary['classes'][] = _evoc_query_for_class_description($model, $uri);
  }

  // Add description of each property to $vocabulary['classes']
  $property_uris = _evoc_query_for_property_uris($model, $ns_uri);
  foreach ($property_uris as $uri) {
    if (substr($uri, 0, strlen($ns_uri)) != $ns_uri) {
      $ignored_uris[] = $uri;
      continue;
    }
    $vocabulary['properties'][] = _evoc_query_for_property_description($model, $uri);
  }

  // Add counts of classes and properties whose URIs don't begin with $ns_uri to $vocabulary['ignored_ns']
  foreach ($ignored_uris as $uri) {
    if (!preg_match('!(.*[#/])[^#/]*$!', $uri, $match)) continue;
    $ns = $match[1];
    if (!isset($vocabulary['ignored_ns'][$ns])) {
      $vocabulary['ignored_ns'][$ns] = 0;
    }
    $vocabulary['ignored_ns'][$ns]++;
  }

  // Prefixes and URIs of referenced external namespaces that are not yet known to the system
  $vocabulary['undefined_ns'] = _evoc_import_get_new_namespaces();

  return $vocabulary;
}

// ------ private helper functions ------------------------------------------

function _evoc_query_for_title(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/title', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/title', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/title', '?'),
      array($uri, 'http://purl.org/dc/terms/title', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_query_for_description(&$model, $ns_uri) {
  $queries = array(
    array($ns_uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
    array($ns_uri, 'http://purl.org/dc/elements/1.1/description', '?'),
    array($ns_uri, 'http://purl.org/dc/terms/description', '?'),
  );
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries = array_merge($queries, array(
      array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
      array($uri, 'http://purl.org/dc/elements/1.1/description', '?'),
      array($uri, 'http://purl.org/dc/terms/description', '?'),
    ));
  }
  return _evoc_query_find_literal($model, $queries);
}

function _evoc_query_for_class_uris(&$model) {
  return _evoc_query_find_uris($model, array(
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2000/01/rdf-schema#Class'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#Class'),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#subClassOf', null),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#subClassOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#disjointWith', null),
    array(null, 'http://www.w3.org/2002/07/owl#disjointWith', '?'),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#domain', '?'),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#range', '?'),
  ));
}

function _evoc_query_for_property_uris(&$model) {
  return _evoc_query_find_uris($model, array(
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#Property'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#DatatypeProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ObjectProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#SymmetricProperty'),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#domain', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#range', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', null),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', null),
    array(null, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
  ));
}

function _evoc_query_for_term_description(&$model, $uri) {
  $label = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#label', '?')
  ));

  $comment = _evoc_query_find_literal($model, array(
    array($uri, 'http://www.w3.org/2004/02/skos/core#definition', '?'),
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#comment', '?'),
  ));

  $qname = _evoc_uri_to_qname($uri);
  return array(
      'uri' => $uri,
      'qname' => $qname,
      'id' => substr($qname, strpos($qname, ':') + 1),
      'prefix' => substr($qname, 0, strpos($qname, ':')),
      'label' => $label,
      'comment' => $comment,
  );
}

function _evoc_query_for_class_description(&$model, $uri) {
  $class = _evoc_query_for_term_description($model, $uri);
  $class['superclasses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subClassOf', '?')
  ));
  $class['disjoints'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#disjointWith', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#disjointWith', $uri),
  ));
  return $class;
}

function _evoc_query_for_property_description(&$model, $uri) {
  $property = _evoc_query_for_term_description($model, $uri);
  $property['domains'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#domain', '?')
  ));
  $property['ranges'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#range', '?')
  ));
  $property['superproperties'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?')
  ));
  $property['is_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#FunctionalProperty')
  ));
  $property['is_inverse_functional'] = _evoc_query_ask($model, array(
    array($uri, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#InverseFunctionalProperty')
  ));
  $property['inverses'] = _evoc_query_find_qnames($model, array(
    array($uri, 'http://www.w3.org/2002/07/owl#inverseOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#inverseOf', $uri),
  ));
  // @todo TODO We don't handle multiple inverses correctly everywhere yet, so store it both as an array and singleton
  $property['inverse'] = count($property['inverses']) ? $property['inverses'][0] : null;
  return $property;
}

// @todo TODO RAP-specific stuff stats here, port to ARC2! ---------------
function _evoc_query_ask(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    if (_evoc_query_find_first($model, $s, $p, $o)) return true;
  }
  return false;
}

function _evoc_query_find_literal(&$model, $queries) {
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $stmt = _evoc_query_find_first($model, $s, $p, $o);
    if (!$stmt) continue;
    if ($s == '?' && is_a($stmt->getSubject(), 'Literal')) {
      return $stmt->getSubject()->getLabel();
    }
    if ($p == '?' && is_a($stmt->getPredicate(), 'Literal')) {
      return $stmt->getPredicate()->getLabel();
    }
    if ($o == '?' && is_a($stmt->getObject(), 'Literal')) {
      return $stmt->getObject()->getLabel();
    }
  }
  return null;
}

function _evoc_query_find_uris(&$model, $queries) {
  $uris = array();
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $result = _evoc_query_find_all($model, $s, $p, $o);
    foreach ($result as $stmt) {
      if ($s == '?' && is_a($stmt->getSubject(), 'Resource') && !is_a($stmt->getSubject(), 'Blanknode')) {
        $uris[] = $stmt->getSubject()->getURI();
      }
      if ($p == '?' && is_a($stmt->getPredicate(), 'Resource') && !is_a($stmt->getPredicate(), 'Blanknode')) {
        $uris[] = $stmt->getPredicate()->getURI();
      }
      if ($o == '?' && is_a($stmt->getObject(), 'Resource') && !is_a($stmt->getObject(), 'Blanknode')) {
        $uris[] = $stmt->getObject()->getURI();
      }
    }
  }
  return array_unique($uris);
}

function _evoc_query_find_qnames(&$model, $queries) {
  $uris = _evoc_query_find_uris($model, $queries);
  $qnames = array();
  foreach ($uris as $uri) {
    $qnames[] = _evoc_uri_to_qname($uri);
  }
  return $qnames;
}

function _evoc_to_rap_resource($s) {
  if (is_null($s)) return null;
  if ($s == '?') return null;
  return new Resource($s);
}

function _evoc_query_find_first(&$model, $s, $p, $o) {
  return $model->findFirstMatchingStatement(_evoc_to_rap_resource($s), _evoc_to_rap_resource($p), _evoc_to_rap_resource($o));
}

function _evoc_query_find_all(&$model, $s, $p, $o) {
  $result = array();
  $it = $model->findAsIterator(_evoc_to_rap_resource($s), _evoc_to_rap_resource($p), _evoc_to_rap_resource($o));
  while ($it->hasNext()) {
    $result[] = $it->next();
  }
  return $result;
}
// @todo TODO RAP-specific stuff ends here ---------------

/**
 * Prepares the importer's namespace tracking.
 *
 * @param string $ns_uri Namespace URI of the vocabulary being imported
 * @param string $ns_prefix Namespace prefix of the vocabulary being imported
 * @param object $model RDF model containing the vocabulary file (including parsed namespaces)
 */
function _evoc_prepare_import_namespaces($ns_uri, $ns_prefix, &$model) {
  global $evoc_import_namespaces;
  $evoc_import_namespaces = array();
  $prefixes = array();
  $namespaces = rdf_get_namespaces();
  foreach ($namespaces as $p => $u) {
    if (is_array($u)) {
      $u = $u[0];
    }
    $evoc_import_namespaces[$u] = array('prefix' => $p);
    $prefixes[] = $p;
  }
  $evoc_import_namespaces[$ns_uri] = array('prefix' => $ns_prefix, 'importing' => true);
  rdf_register_namespace($ns_prefix, $ns_uri);
  $prefixes[] = $ns_prefix;
  $namespaces = $model->getParsedNamespaces();
  foreach ($namespaces as $u => $p) {
    if (!isset($evoc_import_namespaces[$u]) && !in_array($p, $prefixes)) {
      $evoc_import_namespaces[$u] = array('prefix' => $p, 'new' => true);
      rdf_register_namespace($p, $u);
    }
  }
}

/**
 * Abbreviates a URI into a QName, taking into account any namespaces
 * registered by modules, and any namespaces defined in the RDF model
 * currently being imported. Will generate new namespace mappings if
 * necessary.
 *
 * @param string $uri
 * @return string
 */
function _evoc_uri_to_qname($uri) {
  global $evoc_import_namespaces;
  $longest_match = '';
  foreach ($evoc_import_namespaces as $ns_uri => $ns_prefix) {
    if (substr($uri, 0, strlen($ns_uri)) == $ns_uri && strlen($ns_uri) > strlen($longest_match)) {
      $longest_match = $ns_uri;
    }
  }
  if ($longest_match) {
    $qname = $evoc_import_namespaces[$longest_match]['prefix'] . ':' . substr($uri, strlen($longest_match));
    $evoc_import_namespaces[$longest_match]['used'] = true;
    return $qname;
  }
  // The prefix is not known. Split it into NS and local.
  if (!preg_match('!^(.*[:/#])[^:/#]*$!', $uri, $match)) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $ns = $match[1];
  // Use the RDF API to generate a namespace prefix for us (g.1, g.2, etc)
  // Note: We do the splitting ourselves above because the RDF API function
  // doesn't have the proper regex, it doesn't allow certain characters such
  // as dots in the local name, resulting in funky namespace URIs such as
  // <http://...../foo.owl#Foo.>.
  $qname = rdf_uri_to_qname($ns . 'local');
  if (!$qname) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $evoc_import_namespaces[$ns]['prefix'] = substr($qname, 0, strpos($qname, ':'));
  $evoc_import_namespaces[$ns]['new'] = true;
  $evoc_import_namespaces[$ns]['used'] = true;
  return $qname;
}

/**
 * Returns an associative array from prefixes to URIs that contains
 * any namespaces that are necessary for the imported vocabulary,
 * but were not yet previously known to the rdf module. The prefixes
 * will come from the imported RDF model if possible, or are
 * auto-generated otherwise.
 *
 * @return array
 */
function _evoc_import_get_new_namespaces() {
  global $evoc_import_namespaces;
  $result = array();
  foreach ($evoc_import_namespaces as $ns_uri => $x) {
    if ($x['used'] && $x['new']) {
      $result[$x['prefix']] = $ns_uri;
    }
  }
  return $result;
}

?>
