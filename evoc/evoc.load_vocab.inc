<?php

// evoc value and ARC2 parser name
define("EVOC_NO_PARSER", 0);
define("EVOC_RDFXML_PARSER", "RDFXML");
define("EVOC_TURTLE_PARSER", "Turtle");
define("EVOC_RDFA_PARSER", "SemHTML");
define("EVOC_DETECT_PARSER", "detect");

/**
 * Loads an RDF/XML file from an HTTP URI, parses it, and builds
 * an RDF schema representation (associative array) from it.
 * Handles content negotiation and various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On network or parse error
 */
function evoc_get_from_web($url, $uri, $prefix) {
  $schema_url = $url != NULL ? $url : $uri;
  
  if ($schema_url == NULL) {
    throw new Exception('An URL where the resource is located must be provided.');
  }
  
  $schema_url = trim($schema_url);
  // check if it's an HTTP URI
  if (!preg_match('!^http://!', $schema_url)) {
    throw new Exception('Can only read from http:// URLs');
  }
  
  $i = strpos($schema_url, '#');
  if ($i !== false) {
    $schema_url = substr($schema_url, 0, $i);
  }
  
  $reader = _evoc_get_stream_reader();
  if (!$reader) {
    throw new Exception('Unable to get a Reader for request operation.');
  }
  
  try {
    $result = _evoc_activate_connection_and_parse_content($reader, $schema_url);
  } catch (Exception $e) {
    throw $e;  
  }
  
  return _evoc_extract_schema($result->triples, $result->namespaces, $prefix, $uri);
}

/**
 * Loads a local RDF/XML file, parses it, and builds
 * an RDF schema representation (associative array) from it.
 * Handles various errors.
 *
 * @param string $uri
 * @return array
 * @throws Exception On file or parse error
 */
function evoc_get_from_file($filename, $uri, $prefix) {
  //throw new Exception("This feature is not implemented yet!");
  
  if (!is_file($filename)) {
    throw new Exception("File not found: '$filename'");
  }
  
//  $reader = _evoc_get_stream_reader();
//  if (!$reader) {
//    throw new Exception('Unable to get a Reader for request operation.');
//  }
//  
//  $reader->activate($filename);
//  $format = $reader->getFormat();
//  $headers = $reader->getResponseHeaders();
//  $content_type = $headers['content-type'];
  
//  dpr($format);
//  dpr($headers);
//  dpr($content_type);
//  dpr($reader);
//  die();
  
//  try {
//    $result = _evoc_activate_connection_and_parse_content($reader, $filename);
//  } catch (Exception $e) {
//    throw $e;  
//  }
//  
//  return _evoc_extract_schema($result->triples, $result->namespaces, $prefix, $uri);
  
  $content = _evoc_fetch_from_file($filename);
  
  $extension = _evoc_findexts($filename);
  if ($extension == 'rdf') {
    list($triples, $namespaces) = _evoc_parse_rdfxml(NULL, $content);
    return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);  
  }
  else if ($extension == 'html') {
    list($triples, $namespaces) = _evoc_parse_rdfa(NULL, $content);
    return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);  
  }
  else if ($extension == 'ttl') {
    list($triples, $namespaces) = _evoc_parse_turtle(NULL, $content);
    return _evoc_extract_schema($triples, $namespaces, $prefix, $uri);  
  }
  throw new Exception('Cannot select a parser...');
}

/**
 * Loads the content of a local file.
 *
 * @param string $filename
 * @return string
 * @throws Exception
 */
function _evoc_fetch_from_file($filename) {
//  if (!is_file($filename)) {
//    throw new Exception("File not found: '$filename'");
//  }
  
//  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
//  $reader = ARC2::getComponent('Reader');
//  $reader->activate($filename);
//  $format = $reader->getFormat();
//  $headers = $reader->getResponseHeaders();
//  $content_type = $headers['content-type'];
//  
//  dpr($reader);
//  die();
  
  //return file_get_contents($filename);
}

function _evoc_get_stream_reader() {
  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
  $reader = ARC2::getComponent('Reader');
  return $reader; 
}

function _evoc_activate_connection_and_parse_content($reader, $url) {
  //_evoc_activate_connection_and_get_parser($reader);
  $reader->activate($url);
  if (($error = _evoc_arc2_component_get_errors($reader))) {
    throw new Exception($error);  
  }
  
  $rdf_content = NULL;
  $format = $reader->getFormat();
  $headers = $reader->getResponseHeaders();
  $content_type = $headers['content-type'];
  if (isset($content_type)) {
    $len = strcspn($content_type, '; ');
    $media_type = substr($headers['content-type'], 0, $len);  
  }
  
  $parser_type = EVOC_NO_PARSER;
  switch ($media_type) {
    case 'application/rdf+xml':
    // rdf old mediatypes
    case 'text/rdf':
    case 'application/rdf':  
      $parser_type = EVOC_RDFXML_PARSER;
      break;
      
    case 'text/turtle':
    //case 'text/plain':
    case 'text/rdf+n3': // old Neologism n3
      $parser_type = EVOC_TURTLE_PARSER;
      break;
      
    case 'text/html':
    case 'application/xhtml+xml':
      $parser_type = EVOC_RDFA_PARSER;
      break;
      
    // too generic 
    case 'text/plain':
    default:
      if ($format == 'rdfxml') {
        $parser_type = EVOC_RDFXML_PARSER;  
      }
      if ($format == 'turtle') {
        $parser_type = EVOC_TURTLE_PARSER;  
      }
      if ($format == 'html') {
        $parser_type = EVOC_RDFA_PARSER;  
      }
      break;
  }
  
  $parser_conf = array($parser_type, $media_type, $format);
  return _evoc_parse_content($parser_conf, $url, $rdf_content);  
}

function _evoc_parse_content(array $parser_conf, $url, $content = NULL) {
  list($parser_type, $media_type, $format) = $parser_conf;
  
  if ($parser_type !== EVOC_NO_PARSER) {
    if ($parser_type == EVOC_RDFXML_PARSER) {
      list($triples, $namespaces) = _evoc_parse_rdfxml($url, $content);
    }
    else if ($parser_type == EVOC_TURTLE_PARSER) {
      list($triples, $namespaces) = _evoc_parse_turtle($url, $content);
    }
    else if ($parser_type == EVOC_RDFA_PARSER) {
      list($triples, $namespaces) = _evoc_parse_rdfa($url, $content);
    }
    else if ($parser_type == EVOC_DETECT_PARSER) { // EVOC_DETECT_PARSER. let's execute the generic ARC2 parser
      list($triples, $namespaces) = _evoc_parse_arc2_generic($url, NULL);
    }
    else {
      throw new Exception('Unknow parser...');  
    }
  }
  else {
    if ($format == 'ntriples' || $format == 'n3') {
      throw new Exception("Cannot process $format format. Can only process RDF/XML, Turtle and RDFa formats."); 
    }
    
    throw new Exception("Can only process RDF/XML, Turtle and RDFa formats; reported media type was '$media_type'.");   
  }
  
  $result = new stdClass();
  $result->triples = $triples;
  $result->namespaces = $namespaces;
  
  return $result;  
}

/**
 * Loads an RDF content from an HTTP URL. Select a parser for the found content 
 * and check for various errors. Inspects the server's Content-Type header to 
 * make reasonably sure that it actually is an RDF content.
 * @param $url
 * @return Object type holding three properties:
 *   - arc2_reader: The arc2 based reader
 */
//function _evoc_fetch_from_web($url) {
//  $result = new stdClass();
//   
//  // @todo should use drupal_http_request() instead of file_get_contents+stream?
//  $url = trim($url);
//  // check if it's an HTTP URI
//  if (!preg_match('!^http://!', $url)) {
//    throw new Exception('Can only read from http:// URLs');
//  }
//
//  $schema_url = $url;
//  
//  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
//  $reader = ARC2::getComponent('Reader');
//  //'application/rdf+xml, text/turtle;q=0.8, text/rdf+n3;q=0.7, text/rdf+xml;q=0.1, text/xml;q=0.1, application/xml;q=0.1, text/plain;q=0.1, application/xhtml+xml;q=0.1');
//  //$reader->setCustomHeaders("Accept: x-turtle; q=0.98");
//  $reader->activate($schema_url);
//  if (($error = _evoc_parcer_get_errors($parser))) {
//    throw new Exception($error);  
//  }
//  
//  return $reader;
//}

//function _evoc_detect_parser($reader) {
//  if (!$reader) {
//    throw new Exception("Cannot detect parser with no Reader.");
//  }
//  $result = new stdClass();
//  
//  $format = $reader->getFormat();
//  $headers = $reader->getResponseHeaders();
//  $content_type = $headers['content-type'];
//  if (isset($content_type)) {
//    $len = strcspn($content_type, '; ');
//    $media_type = substr($headers['content-type'], 0, $len);  
//  }
//  
//  $parser_type = EVOC_NO_PARSER;
//  switch ($media_type) {
//    case 'application/rdf+xml':
//    // rdf old mediatypes
//    case 'text/rdf':
//    case 'application/rdf':  
//      $parser_type = EVOC_RDFXML_PARSER;
//      break;
//      
//    case 'text/turtle':
//    //case 'text/plain':
//    case 'text/rdf+n3': // old Neologism n3
//      $parser_type = EVOC_TURTLE_PARSER;
//      break;
//      
//    case 'text/html':
//    case 'application/xhtml+xml':
//      $parser_type = EVOC_RDFA_PARSER;
//      break;
//      
//    // too generic 
//    case 'text/plain':
//    default:
//      if ($format == 'rdfxml') {
//        $parser_type = EVOC_RDFXML_PARSER;  
//      }
//      if ($format == 'turtle') {
//        $parser_type = EVOC_TURTLE_PARSER;  
//      }
//      if ($format == 'html') {
//        $parser_type = EVOC_RDFA_PARSER;  
//      }
//      break;
//  }
//  
//  if ($parser_type === EVOC_NO_PARSER) {
//    if ($format == 'ntriples' || $format == 'n3') {
//      throw new Exception("Cannot process $format format. Can only process RDF/XML, Turtle and RDFa formats."); 
//    }
//    else {
//      throw new Exception("Can only process RDF/XML, Turtle and RDFa formats; reported media type was '$media_type'.");   
//    }  
//  }
//  
//  $result->arc2_reader = $reader;
//  $result->media_type  = $media_type;
//  $result->parser_type = $parser_type;
//
//  return $result; 
//}

/**
 * Check if there are erros in the parser. If it does a message is build with the errors.
 * @param $parcer the ARC2 parser
 * @return 
 *   the built error message 
 */
function _evoc_arc2_component_get_errors($component) {
  $error_message = NULL;
  $errors = $component->getErrors();
  if ($errors) {
		$error_message = '';
		if (is_array($errors)) {
  		foreach ($errors as $error) {
  			$error_message .= $error.'<br/>';
  		}
		}
		else {
		   $error_message = $errors; 
		} 
  }
  return $error_message;
}

function &_evoc_parse_rdfxml($base_uri, $rdf_content) {
  // Remove Byte Order Mark if present
  if (strlen($rdf_content) >= 3 && ord($rdf_content[0]) == 0xEF && ord($rdf_content[1]) == 0xBB && ord($rdf_content[2]) == 0xBF) {
    $rdf_content = substr($rdf_content, 3);
  }
//  if (substr(ltrim($rdf_content), 0, 1) != '<') {
//    throw new Exception("Parse error: File does not start with '<'");
//  }

  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
  $parser = ARC2::getRDFXMLParser();
  $parser->parse($base_uri, $rdf_content);
  if (($error = _evoc_arc2_component_get_errors($parser))) {
    throw new Exception($error);  
  }
  $namespaces = _evoc_retrive_namespaces($parser);
  return array($parser->getTriples(), $namespaces);
}

/**
 * Parse html content an extract RDFa 
 * @param String $base_uri
 *   The basepath uri
 * @param String $html_content
 *   The turtle content/data 
 * @return unknown_type
 */
function &_evoc_parse_arc2_generic($base_uri, $content) {
  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
  
  $parser = ARC2::getRDFParser();
  $parser->parse($base_uri, $content);
  if (($error = _evoc_arc2_component_get_errors($parser))) {
    throw new Exception($error);  
  }
  $namespaces = _evoc_retrive_namespaces($parser);
  return array($parser->getTriples(), $namespaces);
}

/**
 * Parse html content an extract RDFa 
 * @param String $base_uri
 *   The basepath uri
 * @param String $html_content
 *   The turtle content/data 
 * @return unknown_type
 */
function &_evoc_parse_rdfa($base_uri, $html_content) {
  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
  
  $config = array('auto_extract' => 0);
  $parser = ARC2::getSemHTMLParser($config);
  $parser->parse($base_uri, $html_content);
  if (($error = _evoc_arc2_component_get_errors($parser))) {
    throw new Exception($error);  
  }
	
  $parser->extractRDF('rdfa');
  $namespaces = _evoc_retrive_namespaces($parser);
  return array($parser->getTriples(), $namespaces);
}

/**
 * Parse turtle content
 * @param String $base_uri
 *   The basepath uri
 * @param String $html_content
 *   The turtle content/data 
 * @return unknown_type
 */
function &_evoc_parse_turtle($base_uri, $html_content) {
  include_once(drupal_get_path('module', 'rdf') . '/vendor/arc/ARC2.php');
  
  $parser = ARC2::getTurtleParser();
	$parser->parse($base_uri, $html_content);
  if (($error = _evoc_arc2_component_get_errors($parser))) {
    throw new Exception($error);  
  }
  
	$namespaces = _evoc_retrive_namespaces($parser);
  return array($parser->getTriples(), $namespaces);
}

/**
 * Retrieve the namespaces from a parser result
 * @param $parser_result
 * @return 
 *   namespaces array
 */
function _evoc_retrive_namespaces($parser) {
  $namespaces = array();
  if (!empty($parser->nsp)) {
    foreach ($parser->nsp as $uri => $prefix) {
      // to fix the warning for strpos() at modules/rdf/rdf.api.inc on line 358
      if (empty($prefix)) {
        $prefix = "?";  
      }
      $namespaces[$prefix] = $uri;
    }  
  }
  return $namespaces;
}

function _evoc_load__error_handler($errno, $errstr) {
    throw new Exception($errstr);
}

/**
 * Queries a set of triples for classes and properties, and builds
 * an associative array describing the vocabulary and any 
 * classes and properties found.
 *
 * @param array $model An ARC2-style array of triples an RDFS vocabulary or OWL ontology
 * @param array $namespaces Associative array of namespaces parsed from the RDF file
 * @param string $ns_prefix Namespace prefix for the vocabulary
 * @param string $ns_uri Only terms in this namespace will be considered
 * @return array Array describing the vocabulary, its classes and properties.
 */
function _evoc_extract_schema(&$model, $namespaces, $ns_prefix, $ns_uri) {

  _evoc_prepare_import_namespaces($ns_uri, $ns_prefix, $namespaces);

  $title = _evoc_query_for_title($model, $ns_uri);
  if ($title == NULL) {
    $vocabulary_uri = _evoc_query_for_vocabulary_uri($model);
    if ($vocabulary_uri) {
      $title = _evoc_query_for_title($model, $vocabulary_uri);
    }
  }
  
  $description = _evoc_query_for_description($model, $ns_uri);
  if ($description == NULL) {
    if ( !isset($vocabulary_uri) ) {
      $vocabulary_uri = _evoc_query_for_vocabulary_uri($model);
    }
    if ($vocabulary_uri) {
      $description = _evoc_query_for_description($model, $vocabulary_uri);
    }
  }
 
  $vocabulary = array(
      'prefix' => $ns_prefix,
      'uri' => $ns_uri,
      'title' => $title,
      'description' => $description,
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
    array('?', 'http://www.w3.org/2002/07/owl#equivalentClass', null),
    array(null, 'http://www.w3.org/2002/07/owl#equivalentClass', '?'),
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
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AnnotationProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#ReflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#IrreflexiveProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#AsymmetricProperty'),
    array('?', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', 'http://www.w3.org/2002/07/owl#TransitiveProperty'),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#domain', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#range', null),
    array('?', 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', null),
    array(null, 'http://www.w3.org/2000/01/rdf-schema#subPropertyOf', '?'),
    array('?', 'http://www.w3.org/2002/07/owl#equivalentProperty', null),
    array(null, 'http://www.w3.org/2002/07/owl#equivalentProperty', '?'),
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
    $triple = _evoc_query_find_first($model, $s, $p, $o);
    if (!$triple) continue;
    if ($triple['o_type'] == 'literal') return $triple['o'];
  }
  return null;
}

function _evoc_query_find_uris(&$model, $queries) {
  $uris = array();
  foreach ($queries as $query) {
    list($s, $p, $o) = $query;
    $result = _evoc_query_find_all($model, $s, $p, $o);
    foreach ($result as $triple) {
      if ($s == '?' && $triple['s_type'] == 'uri') {
        $uris[] = $triple['s'];
      }
      if ($p == '?') {
        $uris[] = $triple['p'];
      }
      if ($o == '?' && $triple['o_type'] == 'uri') {
        $uris[] = $triple['o'];
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

function _evoc_query_find_first(&$model, $s, $p, $o) {
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s || $triple['s_type'] != 'uri')) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o || $triple['o_type'] != 'uri')) continue;
    return $triple;
  }
  return null;
}

function _evoc_query_find_all(&$model, $s, $p, $o) {
  $result = array();
  foreach ($model as $triple) {
    if (!is_null($s) && $s != '?' && ($triple['s'] != $s || $triple['s_type'] != 'uri')) continue;
    if (!is_null($p) && $p != '?' && ($triple['p'] != $p)) continue;
    if (!is_null($o) && $o != '?' && ($triple['o'] != $o || $triple['o_type'] != 'uri')) continue;
    $result[] = $triple;
  }
  return $result;
}

/**
 * Prepares the importer's namespace tracking.
 *
 * @param string $ns_uri Namespace URI of the vocabulary being imported
 * @param string $ns_prefix Namespace prefix of the vocabulary being imported
 * @param array $parsed_namespaces Associative array containing the namespaces parsed from the RDF file
 */
function _evoc_prepare_import_namespaces($ns_uri, $ns_prefix, $parsed_namespaces) {
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
  foreach ($parsed_namespaces as $u => $p) {
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

/**
 * Return the vocabulary page uri for vocabularies that have custom namespace uri.
 * @param unknown_type $model 
 * @param unknown_type $ns_uri This field is not used at the moment but it has been added for future use.
 * @return unknown_type
 */
function _evoc_query_for_vocabulary_uri(&$model, $ns_uri = NULL) {
  $triple = _evoc_query_find_first($model, '?', 'http://purl.org/vocab/vann/preferredNamespaceUri', '?');
  if ($triple) {
    return $triple['s'];
  }
  return NULL;
}

/**
 * Find the extensions of a resource and return the last one found.
 * @param unknown_type $uri
 * @return unknown_type
 */
function _evoc_findexts($uri) { 
  $exts = explode(".", $uri);
  $n = count($exts)-1; 
  $exts = $exts[$n]; 
  $exts = strtolower($exts) ; 
  return $exts; 
} 

/**
 * Check for URI dependencies in the term's properties
 * @param $array_term_values 
 *   The term's properties
 * @param $array_ns
 *   Array of undefined namespaces
 * @return 
 *   Array of found dependencies in the form term_qname => [$qname, ns_uri]
 */
function _evoc_check_vocabulary_dependency(array $array_term_values, array $array_ns) {
  $dependencies = array();
  
  $prefix       = $array_term_values['prefix'];
  $id           = $array_term_values['id'];
  $superclasses = $array_term_values['superclasses'];
  $term_qname   = $prefix.':'.$id;

  // check for superclasses dependencies
  if (count($superclasses)) {
    foreach ($superclasses as $qname) {
      $term = explode(':', $qname);
      if ($prefix != $term[0]) {
        foreach ($array_ns as $ns_prefix => $ns_uri) {
          if ($term[0] == $ns_prefix) {
            $dependencies[$term_qname] = array($qname, $ns_uri);
            break;   
          }
        }
      }
    } 
  }
  
  if (count($array_term_values['superproperties'])) {
    foreach ($array_term_values['superproperties'] as $qname) {
      $term = explode(':', $qname);
      if ($prefix != $term[0]) {
        foreach ($array_ns as $ns_prefix => $ns_uri) {
          if ($term[0] == $ns_prefix) {
            $dependencies[$term_qname] = array($qname, $ns_uri);
            break;   
          }
        }
      }
    } 
  }
  
  return $dependencies;
}

/**
 * Find the terms beloging to each undefined namespaces.
 * @param array $array_undefined_ns
 * @param unknown_type $vocabulary
 * @return array of two elements holding the candidate namespace and the 
 *   dependencies. e.g:  
 *   ns_candidates = array(2) { 
 *     ["http://purl.org/NET/scovo#"]=> int(8) 
 *     ["http://purl.org/NET/c4dm/event.owl#"]=> int(1) 
 *   } 
 *   dependencies = array(1) { 
 *     ["http://purl.org/NET/scovo#"]=> array(1) { 
 *       [0]=> array(1) { 
 *         [0]=> array(1) { 
 *           ["g.2:local"]=> string(35) "http://purl.org/NET/c4dm/event.owl#" 
 *         } 
 *       } 
 *     } 
 *   }
 */
function evoc_get_terms_of_undefined_namespaces(array $array_undefined_ns, $vocabulary) {
  $ns_candidates = array();
  $dependencies  = array();
  
  foreach ($array_undefined_ns as $prefix => $uri) {
    if (count($vocabulary['classes'])) {
      foreach ($vocabulary['classes'] as $index => $term_values)  {
        if ($term_values['prefix'] == $prefix) {
          $ns_candidates[$uri]++;    
          $d = _evoc_check_vocabulary_dependency($term_values, $vocabulary['undefined_ns']);
          if (!empty($d)) {
            $dependencies[$uri][] = $d; 
          }
        }
      }
    }
    
    if (count($vocabulary['properties'])) {
      foreach ($vocabulary['properties'] as $index => $term_values)  {
        if ($term_values['prefix'] == $prefix) {
          $ns_candidates[$uri]++;  
          $d = _evoc_check_vocabulary_dependency($term_values, $vocabulary['undefined_ns']);
          if (!empty($d)) {
            $dependencies[$uri][] = $d;
          }  
        }
      }
    }
  }
  
  return array($ns_candidates, $dependencies);
}

?>
