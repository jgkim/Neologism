<?php

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
function neologism_get_vocabulary_description(&$model, $ns_prefix, $ns_uri) {

  _neologism_prepare_import_namespaces($ns_uri, $ns_prefix, $model);

  $vocabulary = array(
      'prefix' => $ns_prefix,
      'uri' => $ns_uri,
      'title' => _neologism_query_for_title($model, $ns_uri),
      'description' => _neologism_query_for_description($model, $ns_uri),
      'classes' => array(),
      'properties' => array(),
      'ignored_ns' => array(),
      'undefined_ns' => array(),
  );

  // Add description of each class to $vocabulary['classes']
  $class_uris = _neologism_query_for_class_uris($model, $ns_uri);
  $ignored_uris = array();
  foreach ($class_uris as $uri) {
    if (substr($uri, 0, strlen($ns_uri)) != $ns_uri) {
      $ignored_uris[] = $uri;
      continue;
    }
    $vocabulary['classes'][] = _neologism_query_for_class_description($model, $uri);
  }

  // Add description of each property to $vocabulary['classes']
  $property_uris = _neologism_query_for_property_uris($model, $ns_uri);
  foreach ($property_uris as $uri) {
    if (substr($uri, 0, strlen($ns_uri)) != $ns_uri) {
      $ignored_uris[] = $uri;
      continue;
    }
    $vocabulary['properties'][] = _neologism_query_for_property_description($model, $uri);
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
  $vocabulary['undefined_ns'] = _neologism_import_get_new_namespaces();

  return $vocabulary;
}

// ------ private helper functions ------------------------------------------

function _neologism_query_for_title(&$model, $ns_uri) {
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
  return _neologism_sparql_get_first_literal($model, '?title', $queries, $prefixes);
}

function _neologism_query_for_description(&$model, $ns_uri) {
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
  return _neologism_sparql_get_first_literal($model, '?description', $queries, $prefixes);
}

function _neologism_query_for_class_uris(&$model) {
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
  return _neologism_sparql_get_uris($model, '?class', $queries, $prefixes);
}

function _neologism_query_for_property_uris(&$model) {
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
  return _neologism_sparql_get_uris($model, '?property', $queries, $prefixes);
}

function _neologism_query_for_term_description(&$model, $uri) {
  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX skos: <http://www.w3.org/2004/02/skos/core#>';
  $query = 'SELECT ?label WHERE { <' . $uri . '> rdfs:label ?label }';
  $label = _neologism_sparql_get_first_literal($model, '?label', $query, $prefixes);

  $query = array(
      'SELECT ?comment WHERE { <' . $uri . '> skos:definition ?comment }',
      'SELECT ?comment WHERE { <' . $uri . '> rdfs:comment ?comment }');
  $comment = _neologism_sparql_get_first_literal($model, '?comment', $query, $prefixes);
  return array('uri' => $uri, 'qname' => _neologism_uri_to_qname($uri), 'label' => $label, 'comment' => $comment);
}

function _neologism_query_for_class_description(&$model, $uri) {
  $class = _neologism_query_for_term_description($model, $uri);

  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX owl: <http://www.w3.org/2002/07/owl#>';

  $query = 'SELECT ?superclass WHERE { <' . $uri . '> rdfs:subClassOf ?superclass }';
  $class['superclasses'] = _neologism_sparql_get_qnames($model, '?superclass', $query, $prefixes);

  $query = array(
      'SELECT ?disjoint WHERE { <' . $uri . '> owl:disjointWith ?disjoint }',
      'SELECT ?disjoint WHERE { ?disjoint owl:disjointWith <' . $uri . '> }');
  $class['disjoints'] = _neologism_sparql_get_qnames($model, '?disjoint', $query, $prefixes);

  return $class;
}

function _neologism_query_for_property_description(&$model, $uri) {
  $property = _neologism_query_for_term_description($model, $uri);

  $prefixes = '
    PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    PREFIX owl: <http://www.w3.org/2002/07/owl#>';

  $query = 'SELECT ?domain WHERE { <' . $uri . '> rdfs:domain ?domain }';
  $property['domains'] = _neologism_sparql_get_qnames($model, '?domain', $query, $prefixes);

  $query = 'SELECT ?range WHERE { <' . $uri . '> rdfs:range ?range }';
  $property['ranges'] = _neologism_sparql_get_qnames($model, '?range', $query, $prefixes);

  $query = 'SELECT ?superproperty WHERE { <' . $uri . '> rdfs:subPropertyOf ?superproperty }';
  $property['superproperties'] = _neologism_sparql_get_qnames($model, '?superproperty', $query, $prefixes);

  $query = 'SELECT ?t WHERE { <' . $uri . '> a ?t . FILTER (?t = owl:FunctionalProperty) }';
  $t = _neologism_sparql_get_uris($model, '?t', $query, $prefixes);
  $property['is_functional'] = count($t) > 0;

  $query = 'SELECT ?t WHERE { <' . $uri . '> a ?t . FILTER (?t = owl:InverseFunctionalProperty) }';
  $t = _neologism_sparql_get_uris($model, '?t', $query, $prefixes);
  $property['is_inverse_functional'] = count($t) > 0;

  $query = array(
    'SELECT ?inverse WHERE { <' . $uri . '> owl:inverseOf ?inverse }',
    'SELECT ?inverse WHERE { ?inverse owl:inverseOf <' . $uri . '> }');
  $property['inverses'] = _neologism_sparql_get_qnames($model, '?inverses', $query, $prefixes);

  return $property;
}

function _neologism_sparql_get_first_literal(&$model, $var, $queries, $prefixes = '') {
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

function _neologism_sparql_get_uris(&$model, $var, $queries, $prefixes = '') {
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

function _neologism_sparql_get_qnames(&$model, $var, $queries, $prefixes = '') {
  $uris = _neologism_sparql_get_uris($model, $var, $queries, $prefixes);
  $qnames = array();
  foreach ($uris as $uri) {
    $qnames[] = _neologism_uri_to_qname($uri);
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
function _neologism_prepare_import_namespaces($ns_uri, $ns_prefix, &$model) {
  global $neologism_import_namespaces;
  $neologism_import_namespaces = array();
  $prefixes = array();
  $namespaces = rdf_get_namespaces();
  foreach ($namespaces as $p => $u) {
    $neologism_import_namespaces[$u] = array('prefix' => $p);
    $prefixes[] = $p;
  }
  $neologism_import_namespaces[$ns_uri] = array('prefix' => $ns_prefix, 'importing' => true);
  rdf_register_namespace($ns_prefix, $ns_uri);
  $prefixes[] = $ns_prefix;
  $namespaces = $model->getParsedNamespaces();
  foreach ($namespaces as $u => $p) {
    if (!isset($neologism_import_namespaces[$u]) && !in_array($p, $prefixes)) {
      $neologism_import_namespaces[$u] = array('prefix' => $p, 'new' => true);
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
function _neologism_uri_to_qname($uri) {
  global $neologism_import_namespaces;
  $longest_match = '';
  foreach ($neologism_import_namespaces as $ns_uri => $ns_prefix) {
    if (substr($uri, 0, strlen($ns_uri)) == $ns_uri && strlen($ns_uri) > strlen($longest_match)) {
      $longest_match = $ns_uri;
    }
  }
  if ($longest_match) {
    $qname = $neologism_import_namespaces[$longest_match]['prefix'] . ':' . substr($uri, strlen($longest_match));
    if (rdf_is_valid_qname($qname)) {
      $neologism_import_namespaces[$longest_match]['used'] = true;
      return $qname;
    }
  }
  $qname = rdf_uri_to_qname($uri);
  if (!$qname) {
    throw new Exception('Could not generate namespace mapping for URI ' . $uri);
  }
  $local = substr($qname, strpos($qname, ':') + 1);
  $ns = substr($uri, 0, -strlen($local));
  $neologism_import_namespaces[$ns]['prefix'] = substr($qname, 0, strpos($qname, ':'));
  $neologism_import_namespaces[$ns]['new'] = true;
  $neologism_import_namespaces[$ns]['used'] = true;
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
function _neologism_import_get_new_namespaces() {
  global $neologism_import_namespaces;
  $result = array();
  foreach ($neologism_import_namespaces as $ns_uri => $x) {
    if ($x['used'] && $x['new']) {
      $result[$x['prefix']] = $ns_uri;
    }
  }
  return $result;
}

?>
