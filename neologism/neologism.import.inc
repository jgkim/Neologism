<?php

define("SPARQL_QUERY_ERROR", "There is a NULL result in the SPARQL query <br/> 
      </br>Please check you ontology file, or create a new entry for the development team at ".
      l('Neologism at Google code','http://code.google.com/p/neologism/issues/list', array('html' => TRUE, 'attributes' => Array('title' => 'http://code.google.com/p/neologism/issues/list')))
      ." and do not forget to attach your ontology file to it.");

/**
 * This is a hack to work around a problem in Safari: File uploads with
 * multipart/form-data often hang when persistent connections are used.
 * So we close the connection when the upload form is loaded.
 */
function neologism_import_form_callback() {
  header('Connection: close');
  return drupal_get_form('neologism_import_form');
}

function neologism_import_form($form_state) {

  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['ns_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace prefix'),
    '#required' => TRUE,
    '#description' => t('This will be used as both the ID and the namespace prefix for the imported vocabulary. It must be a prefix that is not yet in use.'),
    '#size' => 10,
  );
  $form['ns_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace URI'),
    '#required' => TRUE,
    '#description' => t('<i>Only classes and properties in this namespace will be imported!</i> Must end in “/” or “#”.'),
  );
  $form['from_web'] = array(
    '#type' => 'fieldset',
    '#title' => t('Load vocabulary from the Web'),
    '#collapsible' => false,
    '#description' => t('Use this to import a vocabulary from the Web. We will attempt to load an RDFS vocabulary or OWL ontology from the namespace URI.'),
  );
  $form['from_web']['submit_uri'] = array(
    '#type' => 'submit',
    '#value' => t('Import from Web'),
  );
  $form['from_file'] = array(
    '#type' => 'fieldset',
    '#title' => t('Load vocabulary from RDF file'),
    '#collapsible' => false,
    '#description' => t('Use this to import a vocabulary from an RDF file on your computer. Select an RDF file (in RDF/XML format) that contains an RDFS vocabulary or OWL ontology.'),
  );
  $form['from_file']['file_upload'] = array(
    '#type' => 'file',
    '#title' => t('File upload'),
    '#required' => FALSE,
    '#description' => t('Maximum file size is %size MB.', array('%size' => round(file_upload_max_size()/(1024*1024), 0))),
  );
  $form['from_file']['submit_file'] = array(
    '#type' => 'submit',
    '#value' => t('Import from file'),
  );
  
  return $form;
}

function neologism_import_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['ns_prefix'])) {
    $ns_prefix = $form_state['values']['ns_prefix'];
    if (!_neologism_is_valid_id($ns_prefix)) {
      form_set_error('ns_prefix', t('The prefix must start with a letter and can only contain letters, digits, and any of “-_.”'));
      return;
    }
    $namespaces = rdf_get_namespaces();
    if (isset($namespaces[$ns_prefix])) {
      form_set_error('ns_prefix', t('This prefix is alredy in use.'));
      return;
    }
  }
  if (!empty($form_state['values']['ns_uri'])) {
    $ns_uri = $form_state['values']['ns_uri'];
    if (substr($ns_uri, -1) != '#' && substr($ns_uri, -1) != '/') {
      form_set_error('ns_uri', t('The namespace URI must end in “#” or “/”.'));
      return;
    }
  }

  if ($form_state['clicked_button']['#value'] == t('Import from file')) {
    $validators = array(
      'file_validate_extensions' => array('rdf')
    );
    $file = file_save_upload('file_upload', $validators);
    if ($file) {
      $form_state['values']['uploaded_file'] = $file;
    } else {
      form_set_error('upload', t('You must select a file for upload.'));
      return;
    }
  }
}

function neologism_import_form_submit($form, &$form_state) {
  $ns_prefix = trim($form_state['values']['ns_prefix']);
  $ns_uri = trim($form_state['values']['ns_uri']);

  if ($form_state['values']['uploaded_file']) {
    $uploaded_file = $form_state['values']['uploaded_file'];

    include(RDFAPI_INCLUDE_DIR . "RdfAPI.php");
    include(RDFAPI_INCLUDE_DIR . "syntax/RdfSerializer.php");
    
    $model = ModelFactory::getMemModel();
    $model->load($uploaded_file->filepath);
  } else {
    $vocabulary_url = $ns_uri;
    if (substr($vocabulary_url, -1) == '#') {
      $vocabulary_url = substr($vocabulary_url, 0, -1);
    }
    try {
      $model = evoc_read_RDF_from_URI($vocabulary_url);
    } catch (Exception $e) {
      form_set_error('', 'Import failed: ' . $e->getMessage(), 'error');
      return;
    }
  }
  if (_neologism_import_vocabulary($model, $ns_prefix, $ns_uri)) {
    drupal_set_message(t('The vocabulary has been successfully imported.'));
    drupal_goto($ns_prefix);
  } else {
    form_set_error('', t('Import failed.'));
  }
}

function _neologism_prepare_imported_node($content_type, $title) {
  module_load_include('inc', 'node', 'node.pages');
  $node = new StdClass();
  $node->type = $content_type;
  node_object_prepare($node);
  $node->title = $title;
//  $node->language = $GLOBALS['language']->language;
  return $node;
}

function _neologism_save_imported_node($node) {
  node_save($node);
  if (!$node->nid) {
    drupal_set_message(t('Node for '.$node->title.' could not be created.'), 'error');
    return false;
  }
  watchdog('content', '@type: added %title using neologism API.',
      array('@type' => $node->type, '%title' => $node->title), WATCHDOG_NOTICE, l(t('view'), "node/$node->nid"));
  return $node->nid;
}

function _neologism_new_node_vocabulary(array $vocabulary) {
  
  // check if there is some voc with the same id
  $title = db_result(db_query(db_rewrite_sql("select title from {node} where title = '%s' and type = '%s'"), $vocabulary['prefix'], NEOLOGISM_CT_VOCABULARY));
  if ($title && $title == $vocabulary['prefix']) {
    drupal_set_message($vocabulary['prefix'].' '.t(MSG_ID_ALREADY_IN_USE), 'error');
    return null;
  }

  $node = _neologism_prepare_imported_node(NEOLOGISM_CT_VOCABULARY, $vocabulary['prefix']);
  $node->field_title[0]['value'] = isset($vocabulary['title']) ? $vocabulary['title'] : 'Imported vocabulary';
  $node->field_authors[0]['uid'] = $node->uid;
  $node->field_abstract[0]['value'] = check_plain($vocabulary['description']);
  $node->teaser = '<p>This vocabulary has been imported from <a href="' . $vocabulary['uri'] . '">' . $vocabulary['uri'] . '</a>.</p>';
  $node->body = $node->teaser;
  $node->promote = 1;
  return _neologism_save_imported_node($node);
}

function _neologism_new_node_term($vocabulary_nid, $term, $content_type) {
  $local_name = substr($term['qname'], strpos($term['qname'], ':') + 1);
  if (!_neologism_is_valid_id($local_name)) {
    drupal_set_message($local_name.' '.t(MSG_INVALID_ID), 'error');
    return false;
  }
  $title = db_result(db_query(db_rewrite_sql("select title from {content_field_vocabulary} c inner join {node} n on c.nid = n.nid 
    where c.field_vocabulary_nid = %d and title = '%s' and type = '%s'"), $vocabulary_nid, $local_name, $content_type));
  if ($title && $title == $local_name) {
    drupal_set_message($local_name.' '.t(MSG_ID_ALREADY_IN_USE), 'error');
    return false;
  }
  
  $node = _neologism_prepare_imported_node($content_type, $local_name);
  $node->field_vocabulary[0]['nid'] = $vocabulary_nid;
  $node->field_label[0]['value'] = $term['label'];
  $node->field_comment[0]['value'] = $term['comment'];
  return $node;
}

function _neologism_set_field_qnames(&$node, $field_name, $qnames) {
  $values = array();
  if ($qnames) {
    foreach ($qnames as $qname) {
      $values[] = array('evoc_term' => $qname);
    }
  } else {
    $values[] = array('evoc_term' => null);
  }
  $node->$field_name = $values;
}

function _neologism_new_node_class($vocabulary_nid, $class) {
  $node = _neologism_new_node_term($vocabulary_nid, $class, NEOLOGISM_CT_CLASS);
  _neologism_set_field_qnames($node, 'field_superclass2', $class['superclasses']);
  _neologism_set_field_qnames($node, 'field_disjointwith2', $class['disjoints']);
  return _neologism_save_imported_node($node);
}

function _neologism_new_node_property($vocabulary_nid, $property) {
  $node = _neologism_new_node_term($vocabulary_nid, $property, NEOLOGISM_CT_PROPERTY);
  _neologism_set_field_qnames($node, 'field_domain2', $property['domains']);
  _neologism_set_field_qnames($node, 'field_range2', $property['ranges']);
  _neologism_set_field_qnames($node, 'field_superproperty2', $property['superproperties']);
  $node->field_fp[0]['value'] = $property['is_functional'] ? true : null;
  $node->field_ifp[0]['value'] = $property['is_inverse_functional'] ? true : null;
//  _neologism_set_field_qnames($node, 'field_inverse2', $property['inverses']);    
  return _neologism_save_imported_node($node);
}

function _neologism_import_vocabulary(&$model, $ns_prefix, $ns_uri) {

  _neologism_prepare_import_namespaces($ns_uri, $ns_prefix, $model);

  $vocabulary = _neologism_query_for_vocabulary_description($model, $ns_prefix, $ns_uri);

  // Sanity checks over the extracted vocabulary
  if (!$vocabulary['classes'] && !$vocabulary['properties']) {
    drupal_set_message('No classes or properties found. The file does not appear to be an RDF Schema vocabulary or OWL ontology file.', 'error');
    return false;
  }
  $uris = array();
  foreach ($vocabulary['classes'] as $class) {
    $uris[] = $class['uri'];
  }
  foreach ($vocabulary['properties'] as $property) {
    if (in_array($property['uri'], $uris)) {
      drupal_set_message('Error: ' . $intersection[0] . ' is typed both as a class and as a property.', 'error');
      return false;
    }
  }

  // Fix up missing data
  if (empty($vocabulary['title'])) {
    $vocabulary['title'] = strtoupper($ns_prefix) . " Vocabulary";
  }
  foreach ($vocabulary['classes'] as $key => $class) {
    if (empty($class['label']) && preg_match('!:([^:]+)$!', $class['qname'], $match)) {
      $vocabulary['classes'][$key]['label'] = $match[1]; // local part of the URI
    }
  }
  foreach ($vocabulary['properties'] as $key => $property) {
    if (empty($property['label']) && preg_match('!:([^:]+)$!', $property['qname'], $match)) {
      $vocabulary['properties'][$key]['label'] = $match[1]; // local part of the URI
    }
  }

  // Create new nodes for the vocabulary, its classes and properties
  $vocab_nid = _neologism_new_node_vocabulary($vocabulary);
  if (!$vocab_nid) {
    return false;
  }
  foreach ($vocabulary['classes'] as $class) {
    if (!_neologism_new_node_class($vocab_nid, $class)) return false;
  }
  foreach ($vocabulary['properties'] as $property) {
    if (! _neologism_new_node_property($vocab_nid, $property)) return false;
  }

  // Handle undefined namespaces
  foreach (_neologism_import_get_new_namespaces() as $prefix => $uri) {
    drupal_set_message('Missing external vocabulary: ' . $prefix . ' => ' . $uri, 'warning');
  }

  return true;
}

function _neologism_query_for_vocabulary_description(&$model, $ns_prefix, $ns_uri) {
  $vocabulary = array(
      'prefix' => $ns_prefix,
      'uri' => $ns_uri,
      'title' => _neologism_query_for_title($model, $ns_uri),
      'description' => _neologism_query_for_description($model, $ns_uri),
      'classes' => array(),
      'properties' => array(),
  );
  $class_uris = _neologism_query_for_class_uris($model, $ns_uri);
  foreach ($class_uris as $uri) {
    $vocabulary['classes'][] = _neologism_query_for_class_description($model, $uri);
  }
  $property_uris = _neologism_query_for_property_uris($model, $ns_uri);
  foreach ($property_uris as $uri) {
    $vocabulary['properties'][] = _neologism_query_for_property_description($model, $uri);
  }
  return $vocabulary;
}

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

function _neologism_query_for_class_uris(&$model, $ns_uri) {
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
  return _neologism_sparql_get_uris($model, '?class', $queries, $prefixes, $ns_uri);
}

function _neologism_query_for_property_uris(&$model, $ns_uri) {
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
  return _neologism_sparql_get_uris($model, '?property', $queries, $prefixes, $ns_uri);
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

function _neologism_sparql_get_uris(&$model, $var, $queries, $prefixes = '', $namespace = '') {
  if (!is_array($queries)) {
    $queries = array($queries);
  }
  $uris = array();
  foreach ($queries as $query) {
    $result = $model->sparqlQuery($prefixes . "\n" . $query);
    if (!$result) continue;
    foreach ($result as $binding) {
      if (!isset($binding[$var]) || !isset($binding[$var]->uri) || is_a($binding[$var], 'BlankNode')) continue;
      $uri = $binding[$var]->uri;
      if (substr($uri, 0, strlen($namespace)) != $namespace) continue;
      $uris[] = $uri;
    }
  }
  return array_unique($uris);
}

function _neologism_sparql_get_qnames(&$model, $var, $queries, $prefixes = '', $namespace = '') {
  $uris = _neologism_sparql_get_uris($model, $var, $queries, $prefixes, $namespace);
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
