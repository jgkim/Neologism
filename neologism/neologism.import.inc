<?php

/**
 * This file contains code related to Neologism's vocabulary import:
 * the import form, and code that transforms a parsed vocabulary into
 * CCK nodes. The actual parsing is done by code in the evoc module.
 *
 * @todo The form is quite redundant with what's found in evoc
 */

/**
 * This is a hack to work around a problem in Safari: File uploads with
 * multipart/form-data often hang when persistent connections are used.
 * So we close the connection when the upload form is loaded.
 */
function neologism_import_form_callback() {
  // check for the persistent variable whether the external built-in vocabularies have been imported. 
  // If the variable is FALSE then a message is shown. 
  if (!variable_get('evoc_builtin_vocabularies_imported', FALSE)) {
    drupal_set_message('This action is not available because the evoc module has not yet been initialized. '
      .l('Visit the evoc module', 'evoc').' to complete the initialization.','error');
    return theme('status_messages');
  }
  
  header('Connection: close');
  return drupal_get_form('neologism_import_form');
}

function neologism_import_form($form_state) {

  if (!empty($form_state['storage']['invalid_ns_found'])) {
    
    // construct the msg string about the detected URIs
    $array_uri = array();
    $msg = t('The following namespace URIs were found in the resource:');
    $msg .= '<ul>';
    foreach ($form_state['storage']['invalid_ns_found'] as $uri => $term_count) {
      $msg .= "<li>$uri ($term_count " . format_plural($term_count, t('term'), t('terms')) . ')';
      
//      if (isset($form_state['storage']['dependencies']) && !empty($form_state['storage']['dependencies'][$uri])) {
//        $count = count($form_state['storage']['dependencies'][$uri]);
//        $msg .= '<ul>'.format_plural($count, t('External dependency:'), t('External dependencies:')); 
//        foreach ($form_state['storage']['dependencies'][$uri] as $key => $value) {
//          foreach ($value as $term => $dependency) {
//            $term = explode(':', $term);
//            $msg .= '<li>'.$form_state['values']['ns_prefix'].':'.$term[1].' => '.$dependency[1].'</li>';
//          }
//        }
//        $msg .= '</ul>';
//      }
      $msg .= '</li>';
      
      // construct the array of options for the select combobox
      $array_uri[$uri] = $uri;
    }
    $msg .= '</ul>';
    
    $msg .= t('The default namespace URI has been selected depending on the number of terms. In order to select a different namespace URI use the <b>Namespace URI</b> combobox and then press the "Import vocabulary" 
    	button or press the "Cancel" button to return to the main import form.');
    
    $form['detected_ns'] = array(
      '#title' => t(' URIs detected '),
      '#type' => 'fieldset',
      '#description' => $msg
    );
    
    $form['ns_prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Namespace prefix'),
      '#required' => TRUE,
      '#description' => t('Examples: <i>foaf</i>, <i>dc</i>, <i>skos</i>. This will be used as both the ID and the namespace prefix for the imported vocabulary. It must be a prefix that is not yet in use. Max 10 characters.'),
      '#size' => 10,
      '#maxlength' => 10,
      '#default_value' => $form_state['values']['ns_prefix']
    );
    
    $form['ns_uri'] = array(
    	'#type' => 'select', 
  		'#title' => t('Namespace URI'), 
    	'#options' => $array_uri,
    	'#description' => t('<i>Only classes and properties in this namespace will be imported!</i> Must end in “/” or “#”.'),
      '#multiple' => FALSE,
    );
    
    if (isset($form_state['values']['uploaded_file'])) {
      $value =  $form_state['values']['uploaded_file']->filename;  
      $description = t('Filename where the data will be retrieve.');
      $title = t('Filename');
      
      $form['uploaded_file'] = array(
      	'#type' => 'value', 
    		'#value' => $form_state['values']['uploaded_file']
      );
      
    }
    else {
      $value = $form_state['values']['vocabulary_url'];
      $description = t('Resource locator where the data will be retrieve.');
      $title = t('URL');
    }
    
    $form['vocabulary_url'] = array(
      '#type' => 'textfield',
      '#title' => $title,
      '#required' => TRUE,
      '#description' => $description,
    	'#value' => truncate_utf8($value, 60, TRUE, TRUE),
      '#disabled' => TRUE,
      '#size' => 60, 
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Import vocabulary'),
    );
    
    $form['cancel'] = array(
      '#type' => 'button',
      '#value' => t('Cancel'),
      '#executes_submit_callback' => TRUE,
    );
    
    $form_state['storage']['invalid_ns_found'] = array();
  }
  else {
    $form['#attributes']['enctype'] = 'multipart/form-data';
    $form['ns_prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Namespace prefix'),
      '#required' => TRUE,
      '#description' => t('Examples: <i>foaf</i>, <i>dc</i>, <i>skos</i>. This will be used as both the ID and the namespace prefix for the imported vocabulary. It must be a prefix that is not yet in use. Max 10 characters.'),
      '#size' => 10,
      '#maxlength' => 10,
      '#default_value' => $form_state['values']['ns_prefix']
    );
    $form['from_web'] = array(
      '#type' => 'fieldset',
      '#title' => t('Load vocabulary from the Web'),
      '#collapsible' => false,
      '#description' => t('Use this to import a vocabulary from the Web. We will attempt to load an RDFS vocabulary or OWL ontology from the namespace URI.'),
    );
    $form['from_web']['vocabulary_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Vocabulary URL'),
      '#required' => FALSE,
      //'#description' => t(''),
    	//'#default_value' => $form_state['values']['ns_uri']
    );
    $form['from_web']['submit_uri'] = array(
      '#type' => 'submit',
      '#value' => t('Import from Web'),
    );
    $form['from_file'] = array(
      '#type' => 'fieldset',
      '#title' => t('Load vocabulary from RDF file'),
      '#collapsible' => FALSE,
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
  }
  
  return $form;
}

function neologism_import_form_validate($form, &$form_state) {
  if ($form_state && isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == t('Cancel')) {
    drupal_goto('neologism/import');
    return;
  }
  
  module_load_include('inc', 'evoc', 'evoc.load_vocab'); // included for _evoc_findexts()
  $array_valid_extensions = array('rdf', 'ttl', 'html');
  
  if (!empty($form_state['values']['ns_prefix'])) {
    $ns_prefix = trim($form_state['values']['ns_prefix']);
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
  if (!empty($form_state['values']['vocabulary_url'])) {
    $vocabulary_url = trim($form_state['values']['vocabulary_url']);
    
    // if we are using the 'vocabulary_url' filed to hold the filename the check for valid url should not take place.
    if (!isset($form_state['values']['uploaded_file']) && !rdf_is_valid_url($vocabulary_url)) {
      form_set_error('vocabulary_url', t('Invalid Uniform Resource Locator format.'));
      return;
    }
    
    if (!empty($form_state['values']['ns_uri'])) {
      $ns_uri = $form_state['values']['ns_uri'];
    }
  }

  if (!$ns_prefix) {
    return;
  }
  
  if ($form_state['clicked_button']['#value'] == t('Import from file')) {
    $validators = array(
      'file_validate_extensions' => $array_valid_extensions
    );
    $file = file_save_upload('file_upload', $validators);
    if ($file) {
      $form_state['values']['uploaded_file'] = $file;
    } else {
      form_set_error('file_upload', t('You must select a file for upload.'));
      return;
    }
  }

  // Load and parse the RDF file, either from the Web or from uploaded file
  try {
    if ($form_state['values']['uploaded_file']) {
      $uploaded_file = $form_state['values']['uploaded_file'];
      $vocabulary = evoc_get_from_file($uploaded_file->filepath, $ns_uri, $ns_prefix);
      $source_field = 'file_upload';
    } else {
      $vocabulary = evoc_get_from_web($vocabulary_url, $ns_uri, $ns_prefix);
      $source_field = 'vocabulary_url';
    }
  } catch (Exception $e) {
    form_set_error('', 'Import failed: ' . $e->getMessage(), 'error');
    return;
  }
  
  // Sanity checks over the extracted vocabulary
  if (!$vocabulary['classes'] && !$vocabulary['properties']) {
    if (!$vocabulary['ignored_ns']) {
      form_set_error($source_field, t('The resource does not appear to be an RDF Schema vocabulary or OWL ontology resource. No classes or properties were found.'));
      return;
    }
    
    // save ignored uris to show them to the user, maybe one of them could be 
    // the one that the user is looking for import 
    arsort($vocabulary['ignored_ns'], SORT_NUMERIC);
    $form_state['rebuild'] = TRUE;
    $form_state['storage']['invalid_ns_found'] = $vocabulary['ignored_ns'];
    return;
  }
  
  // Handle undefined namespaces
  // TODO: we can check that one of the undefined_ns is the wanted ns_uri and we can import the vocabulary directly.
  if (count($vocabulary['undefined_ns'])) {
    if (!isset($ns_uri)) {
      list($ns_candidates, $dependencies) = evoc_get_terms_of_undefined_namespaces($vocabulary['undefined_ns'], $vocabulary);
      
      arsort($ns_candidates, SORT_NUMERIC);
      // save ignored uris to show them to the user, maybe one of them could be 
      // the one that the user is looking for import 
      $form_state['rebuild'] = TRUE;
      $form_state['storage']['invalid_ns_found'] = $ns_candidates;
      //$form_state['storage']['dependencies'] = $dependencies;
      return;
    }
    else {
      $ignored_ns = count($vocabulary['ignored_ns']);
      $msg = format_plural($ignored_ns, t('The following namespace URI is used in the vocabulary that is being imported.'), 
        t('The following namespace URIs are used in the vocabulary that is being imported.'));
      $msg .= '<ul>';
      foreach ($vocabulary['ignored_ns'] as $uri => $term_count) {
        $msg .= "<li>$uri</li>";	
      }
      $msg .= t('In orden to get a well detailed vocabulary, install all dependencies as external vocabularies in the '.l('External vocabularies/Add new', 'evoc/add').' section.');
      form_set_error('', $msg);
      return;
    }
  }
  
  $uris = array();
  foreach ($vocabulary['classes'] as $class) {
    $uris[] = $class['uri'];
  }
  foreach ($vocabulary['properties'] as $property) {
    if (in_array($property['uri'], $uris)) {
      form_set_error($source_field, 'Unsupported modelling in vocabulary file: ' . $intersection[0] . ' is typed both as a class and as a property.');
      return;
    }
  }

  if ($source_field == 'file_upload') {
    $vocabulary['body'] = '<p>This vocabulary has been imported from a file.</p>';
  } else {
    $vocabulary['body'] = '<p>This vocabulary has been imported from <a href="' . $vocabulary['uri'] . '">' . $vocabulary['uri'] . '</a>.</p>';
  }

  // Validation successful! Store the vocabulary array in $form_state, hope this works!
  $form_state['neologism_processed_vocabulary'] = $vocabulary;
}

function neologism_import_form_submit($form, &$form_state) {

  // Get the vocabulary array from $form_state, should have been put there by import_form_validate()
  if (!is_array($form_state['neologism_processed_vocabulary'])) {
    drupal_set_message('Internal error: neologism_processed_vocabulary is not present');
    return false;
  }
  $vocabulary = $form_state['neologism_processed_vocabulary'];

  // Save the vocabulary.
  if (!_neologism_import_vocabulary($vocabulary)) {
    // The import function should have set a drupal error if it returned false.
    return false;
  }
  drupal_set_message(t('Imported vocabulary with %classes classes and %properties properties.',
      array('%classes' => count($vocabulary['classes']), '%properties' => count($vocabulary['properties']))));

  drupal_goto($vocabulary['prefix']);
}

function _neologism_import_vocabulary($vocabulary) {

  // Fix up missing data
  if (empty($vocabulary['title'])) {    // No vocab title -- add a default
    $vocabulary['title'] = strtoupper($vocabulary['prefix']) . " Vocabulary";
  }
  foreach ($vocabulary['classes'] as $key => $class) { // No class label -- use local part of URI
    if (empty($class['label']) && preg_match('!:([^:]+)$!', $class['qname'], $match)) {
      $vocabulary['classes'][$key]['label'] = $match[1]; // local part of the URI
    }
  }
  foreach ($vocabulary['properties'] as $key => $property) { // No property label -- use local part of URI
    if (empty($property['label']) && preg_match('!:([^:]+)$!', $property['qname'], $match)) {
      $vocabulary['properties'][$key]['label'] = $match[1]; // local part of the URI
    }
  }

  // Create new nodes for the vocabulary, its classes and properties
  $vocab_nid = _neologism_new_node_vocabulary($vocabulary);
  if (!$vocab_nid) return false;
  foreach ($vocabulary['classes'] as $class) {
    if (!_neologism_new_node_class($vocab_nid, $class)) return false;
  }
  foreach ($vocabulary['properties'] as $property) {
    if (!_neologism_new_node_property($vocab_nid, $property)) return false;
  }

  return true;
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
  $prefix = db_result(db_query(db_rewrite_sql("select prefix from {neologism_vocabulary} where prefix = '%s'"), $vocabulary['prefix']));
  if ($prefix && $prefix == $vocabulary['prefix']) {
    drupal_set_message(t("Prefix %prefix is already in use.", array('%prefix' => $vocabulary['prefix'])), 'error');
    return null;
  }

  $node = _neologism_prepare_imported_node(NEOLOGISM_CT_VOCABULARY, isset($vocabulary['title']) ? $vocabulary['title'] : 'Imported vocabulary');
  $node->prefix = $vocabulary['prefix'];
  $node->authors = array($node->uid);
  $node->abstract = check_plain($vocabulary['description']);
  $node->custom_namespace = $vocabulary['uri'];
  $node->teaser = $vocabulary['body'];
  $node->body = $vocabulary['body'];
  $node->promote = 1;
  return _neologism_save_imported_node($node);
}

function _neologism_new_node_term($vocabulary_nid, $term, $content_type) {
  $local_name = substr($term['qname'], strpos($term['qname'], ':') + 1);
  if (!_neologism_is_valid_id($local_name)) {
    drupal_set_message(t("Encountered invalid local name %id.", array('%id' => $local_name)), 'error');
    return false;
  }
  $title = db_result(db_query(db_rewrite_sql("select title from {content_field_vocabulary} c inner join {node} n on c.nid = n.nid 
    where c.field_vocabulary_nid = %d and title = '%s' and type = '%s'"), $vocabulary_nid, $local_name, $content_type));
  if ($title && $title == $local_name) {
    drupal_set_message(t("Encountered duplicate local name %id.", array('%id' => $local_name)), 'error');
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

function _neologism_is_available_prefix() {
  
}
?>
