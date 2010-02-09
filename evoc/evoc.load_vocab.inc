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
    include_once(RDFAPI_INCLUDE_DIR . "RdfAPI.php");
    include_once(RDFAPI_INCLUDE_DIR . "syntax/RdfSerializer.php");
    
    $uri = trim($uri);
    // check if it's an HTTP URI
    if (!preg_match('!^http://!', $uri)) {
        throw new Exception('Can only read from http:// URIs');
    }
    // create a custom context that sends an Accept header that asks for RDF/XML, Turtle or N3
    $context = stream_context_create(array('http' => array(
            'header' => 'Accept: application/rdf+xml, text/rdf+xml, text/xml;q=0.1, application/xml;q=0.1, text/plain;q=0.1',
    )));
    // read file contents
    set_error_handler('evoc_read_RDF_from_URI__error_handler', E_WARNING);
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
    $model = ModelFactory::getDefaultModel();
    set_error_handler('evoc_read_RDF_from_URI__error_handler', E_USER_ERROR);
    try {
        $model->loadFromString($content, 'rdf');
        restore_error_handler();
        return $model;
    } catch (Exception $ex) {
        restore_error_handler();
        throw $ex;
    }
}

function evoc_read_RDF_from_URI__error_handler($errno, $errstr) {
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
  $queries = array();
  $queries[] = 'SELECT ?title WHERE { <' . $ns_uri . '> rdfs:label ?title }';
  $queries[] = 'SELECT ?title WHERE { <' . $ns_uri . '> dc:title ?title }';
  $queries[] = 'SELECT ?title WHERE { <' . $ns_uri . '> dcterms:title ?title }';
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries[] = 'SELECT ?title WHERE { <' . $uri . '> rdfs:label ?title }';
    $queries[] = 'SELECT ?title WHERE { <' . $uri . '> dc:title ?title }';
    $queries[] = 'SELECT ?title WHERE { <' . $uri . '> dcterms:title ?title }';
  }
  $prefixes = '
    prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    prefix dc: <http://purl.org/dc/elements/1.1/>
    prefix dcterms: <http://purl.org/dc/terms/>';
  return _evoc_sparql_get_first_literal($model, '?title', $queries, $prefixes);
}

function _evoc_query_for_description(&$model, $ns_uri) {
  $queries = array();
  $queries[] = 'SELECT ?description WHERE { <' . $ns_uri . '> rdfs:comment ?description }';
  $queries[] = 'SELECT ?description WHERE { <' . $ns_uri . '> dc:description ?description }';
  $queries[] = 'SELECT ?description WHERE { <' . $ns_uri . '> dcterms:description ?description }';
  if (substr($ns_uri, -1) == '#') {
    $uri = substr($ns_uri, 0, -1);
    $queries[] = 'SELECT ?description WHERE { <' . $uri . '> rdfs:comment ?description }';
    $queries[] = 'SELECT ?description WHERE { <' . $uri . '> dc:description ?description }';
    $queries[] = 'SELECT ?description WHERE { <' . $uri . '> dcterms:description ?description }';
  }
  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX dc: <http://purl.org/dc/elements/1.1/>
    PREFIX dcterms: <http://purl.org/dc/terms/>';
  return _evoc_sparql_get_first_literal($model, '?description', $queries, $prefixes);
}

function _evoc_query_for_class_uris(&$model) {
  $queries = array();
  $queries[] = 'SELECT ?class WHERE { ?class a rdfs:Class }';
  $queries[] = 'SELECT ?class WHERE { ?class a owl:Class }';
  $queries[] = 'SELECT ?class WHERE { ?class rdfs:subClassOf ?other }';
  $queries[] = 'SELECT ?class WHERE { ?other rdfs:subClassOf ?class }';
  $queries[] = 'SELECT ?class WHERE { ?class owl:disjointWith ?other }';
  $queries[] = 'SELECT ?class WHERE { ?other owl:disjointWith ?class }';
  $queries[] = 'SELECT ?class WHERE { ?other rdfs:domain ?class }';
  $queries[] = 'SELECT ?class WHERE { ?other rdfs:range ?class }';
  $prefixes = '
    prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    prefix owl: <http://www.w3.org/2002/07/owl#>';
  return _evoc_sparql_get_uris($model, '?class', $queries, $prefixes);
}

function _evoc_query_for_property_uris(&$model) {
  $queries = array();
  $queries[] = 'SELECT ?property WHERE { ?property a rdf:Property }';
  $queries[] = 'SELECT ?property WHERE { ?property a owl:DatatypeProperty }';
  $queries[] = 'SELECT ?property WHERE { ?property a owl:ObjectProperty }';
  $queries[] = 'SELECT ?property WHERE { ?property a owl:FunctionalProperty }';
  $queries[] = 'SELECT ?property WHERE { ?property a owl:InverseFunctionalProperty }';
  $queries[] = 'SELECT ?property WHERE { ?property a owl:SymmetricProperty }';
  $queries[] = 'SELECT ?property WHERE { ?property rdfs:domain ?other }';
  $queries[] = 'SELECT ?property WHERE { ?property rdfs:range ?other }';
  $queries[] = 'SELECT ?property WHERE { ?property rdfs:subPropertyOf ?other }';
  $queries[] = 'SELECT ?property WHERE { ?other rdfs:subPropertyOf ?property }';
  $queries[] = 'SELECT ?property WHERE { ?property owl:inverseOf ?other }';
  $queries[] = 'SELECT ?property WHERE { ?other owl:inverseOf ?property }';
  $prefixes = '
    prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
    prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    prefix owl: <http://www.w3.org/2002/07/owl#>
    prefix foaf: <http://xmlns.com/foaf/0.1/>
    prefix dc: <http://purl.org/dc/elements/1.1/>
    prefix dcterms: <http://purl.org/dc/terms/>';
  return _evoc_sparql_get_uris($model, '?property', $queries, $prefixes);
}

function _evoc_query_for_term_description(&$model, $uri) {
  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX skos: <http://www.w3.org/2004/02/skos/core#>';
  $query = 'SELECT ?label WHERE { <' . $uri . '> rdfs:label ?label }';
  $label = _evoc_sparql_get_first_literal($model, '?label', $query, $prefixes);

  $query = array(
      'SELECT ?comment WHERE { <' . $uri . '> skos:definition ?comment }',
      'SELECT ?comment WHERE { <' . $uri . '> rdfs:comment ?comment }');
  $comment = _evoc_sparql_get_first_literal($model, '?comment', $query, $prefixes);
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

  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX owl: <http://www.w3.org/2002/07/owl#>';

  $query = 'SELECT ?superclass WHERE { <' . $uri . '> rdfs:subClassOf ?superclass }';
  $class['superclasses'] = _evoc_sparql_get_qnames($model, '?superclass', $query, $prefixes);

  $query = array(
      'SELECT ?disjoint WHERE { <' . $uri . '> owl:disjointWith ?disjoint }',
      'SELECT ?disjoint WHERE { ?disjoint owl:disjointWith <' . $uri . '> }');
  $class['disjoints'] = _evoc_sparql_get_qnames($model, '?disjoint', $query, $prefixes);

  return $class;
}

function _evoc_query_for_property_description(&$model, $uri) {
  $property = _evoc_query_for_term_description($model, $uri);

  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX owl: <http://www.w3.org/2002/07/owl#>';

  $query = 'SELECT ?domain WHERE { <' . $uri . '> rdfs:domain ?domain }';
  $property['domains'] = _evoc_sparql_get_qnames($model, '?domain', $query, $prefixes);

  $query = 'SELECT ?range WHERE { <' . $uri . '> rdfs:range ?range }';
  $property['ranges'] = _evoc_sparql_get_qnames($model, '?range', $query, $prefixes);

  $query = 'SELECT ?superproperty WHERE { <' . $uri . '> rdfs:subPropertyOf ?superproperty }';
  $property['superproperties'] = _evoc_sparql_get_qnames($model, '?superproperty', $query, $prefixes);

  $query = 'SELECT ?t WHERE { <' . $uri . '> a ?t . FILTER (?t = owl:FunctionalProperty) }';
  $t = _evoc_sparql_get_uris($model, '?t', $query, $prefixes);
  $property['is_functional'] = count($t) > 0;

  $query = 'SELECT ?t WHERE { <' . $uri . '> a ?t . FILTER (?t = owl:InverseFunctionalProperty) }';
  $t = _evoc_sparql_get_uris($model, '?t', $query, $prefixes);
  $property['is_inverse_functional'] = count($t) > 0;

  $query = array(
    'SELECT ?inverse WHERE { <' . $uri . '> owl:inverseOf ?inverse }',
    'SELECT ?inverse WHERE { ?inverse owl:inverseOf <' . $uri . '> }');
  // We don't handle multiple inverses correctly everywhere yet, so store it both as an array and singleton
  $property['inverses'] = _evoc_sparql_get_qnames($model, '?inverses', $query, $prefixes);
  $property['inverse'] = count($property['inverses']) ? $property['inverses'][0] : null;

  return $property;
}

function _evoc_sparql_get_first_literal(&$model, $var, $queries, $prefixes = '') {
  if (!is_array($queries)) {
    $queries = array($queries);
  }
  foreach ($queries as $query) {
    $result = $model->sparqlQuery($prefixes . "\n" . $query);
    if (!$result || !isset($result[0]) || !isset($result[0][$var])) continue;
    return $result[0][$var]->label;
  }
  return null;
}

function _evoc_sparql_get_uris(&$model, $var, $queries, $prefixes = '') {
  if (!is_array($queries)) {
    $queries = array($queries);
  }
  $uris = array();
  foreach ($queries as $query) {
    $result = $model->sparqlQuery($prefixes . "\n" . $query);
    if (!$result) continue;
    foreach ($result as $binding) {
      if (!isset($binding[$var]) || !isset($binding[$var]->uri) || is_a($binding[$var], 'BlankNode')) continue;
      $uris[] = $binding[$var]->uri;
    }
  }
  return array_unique($uris);
}

function _evoc_sparql_get_qnames(&$model, $var, $queries, $prefixes = '') {
  $uris = _evoc_sparql_get_uris($model, $var, $queries, $prefixes);
  $qnames = array();
  foreach ($uris as $uri) {
    $qnames[] = _evoc_uri_to_qname($uri);
  }
  return $qnames;
}

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
    if (rdf_is_valid_qname($qname)) {
      $evoc_import_namespaces[$longest_match]['used'] = true;
      return $qname;
    }
  }
  $qname = rdf_uri_to_qname($uri);
  if (!$qname) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $local = substr($qname, strpos($qname, ':') + 1);
  $ns = substr($uri, 0, -strlen($local));
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
