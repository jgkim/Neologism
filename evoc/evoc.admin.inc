<?php
// $Id: evoc.admin.inc, v2.0.0.0 2009/10/27 10:09:07 guidocecilio Exp $
/**
 * evoc.admin.inc - Evoc API admin page callbacks.
 *
 * @author Guido Cecilio
 * @copyright Copyright (c) 2009 Guido Cecilio. All rights reserved.
 * @license GPL <http://creativecommons.org/licenses/GPL/2.0/>
 * @package evoc.module
 */

/**
 * default callback for evoc admin section
 * @return 
 */
function evoc_manage_vocabularies() {
  
  if (!variable_get('evoc_builtin_vocabularies_imported', FALSE)) {
    module_load_include('inc', 'evoc', 'evoc.batch_import');
    return drupal_get_form('evoc_import_builtin_vocabularies_form');
  }
  
  $evoc_array_builtin_vocabularies = variable_get('evoc_builtin_vocabularies', NULL);
  if (!$evoc_array_builtin_vocabularies) {
    drupal_set_message('Error retrieving built-in vocabularies\' information from the database.', 'error');
    return theme('status_messages');
  }
  
  // These are the namespaces imported during installation
  $builtins = array();
  foreach ($evoc_array_builtin_vocabularies as $vocab) {
    $builtins[] =  $vocab['prefix'];
  }
  
  // Get namespaces managed by evoc. First get all from the {rdf_namespaces}
  // table, then remove those managed by other modules (neologism). This
  // is a hack; it will fail if any other modules register namespaces into
  // that table
  // @todo Make sure that we get only namespaces imported via evoc
  $result = db_query("SELECT v.prefix, v.uri FROM {rdf_namespaces} v");
  $namespaces = array();
  while ($row = db_fetch_object($result)) {
    $namespaces[$row->prefix] = $row->uri;
  }
  $namespaces = array_diff_key($namespaces, module_invoke('neologism', 'rdf_namespaces'));
  
  $header = array(
      t('Prefix'),
      t('Namespace URI'),
      array('data' => t('Operations')/*, 'colspan' => '2'*/),
  );

  $rows = array();
  foreach ( $namespaces as $prefix => $uri ) {
    $remove = in_array($prefix, $builtins)
        ? t('(builtin)')
        : l(t('remove'), 'evoc/remove/'.$prefix, array('attributes' => array('title' => 'Remove this external vocabulary')));
    $rows[] = array(
        array('data' => $prefix),
        array('data' => $uri),
        array('data' => $remove),
    );
  }

  $output = theme('table', $header, $rows);
  
  return $output;
}

function evoc_add_vocabulary() {
  if (!variable_get('evoc_builtin_vocabularies_imported', FALSE)) {
    drupal_set_message('This action is not available because the evoc module has not yet been initialized. '
      .l('Visit the evoc module', 'evoc').' to complete the initialization.','error');
    return theme('status_messages');
  }
  else return drupal_get_form('evoc_add_vocabulary_form');
}

function evoc_add_vocabulary_form($form_state) {
  $form = array();
  
  $form['help'] = array(
      '#value' => 'Here you can add external vocabularies. This allows you to use their classes and properties in your own vocabularies, for example, as superclasses. We will attempt to load an RDF/XML file containing an RDFS vocabulary or OWL ontology from the namespace URI.',
  );
  $form['ns_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace prefix'),
    '#required' => TRUE,
    '#description' => t('Examples: <i>foaf</i>, <i>dc</i>, <i>skos</i>. It must be a prefix that is not yet in use.'),
    '#size' => 10,
  );
  $form['ns_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace URI'),
    '#required' => TRUE,
    '#description' => t('<i>Only classes and properties in this namespace will be imported!</i> Must end in “/” or “#”.'),
  );
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Submit',
  );
  return $form;
}

function evoc_add_vocabulary_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['ns_prefix'])) {
    $ns_prefix = trim($form_state['values']['ns_prefix']);
    if (!rdf_is_valid_qname($ns_prefix . ':x')) {
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
    $ns_uri = trim($form_state['values']['ns_uri']);
    if (!rdf_is_valid_uri($ns_uri)) {
      form_set_error('ns_uri', t('Invalid namespace URI. A valid example is <i>http://xmlns.com/foaf/0.1/</i>'));
      return;
    }
    
    if (substr($ns_uri, -1) != '#' && substr($ns_uri, -1) != '/') {
      form_set_error('ns_uri', t('The namespace URI must end in "#" or "/".'));
      return;
    }
  }

  if (!$ns_uri || !$ns_prefix) return;
  
  // Load and parse the RDF file
  module_load_include('inc', 'evoc', 'evoc.load_vocab');
  try {
    $vocabulary = evoc_get_from_web($ns_uri, $ns_prefix);
  } catch (Exception $e) {
    form_set_error('', 'Import failed: ' . $e->getMessage(), 'error');
    return;
  }
  
  // Sanity checks over the extracted vocabulary
  if (!$vocabulary['classes'] && !$vocabulary['properties']) {
    if (!$vocabulary['ignored_ns']) {
      form_set_error('ns_uri', t('No classes or properties found. The resource does not appear to be an RDF Schema vocabulary or OWL ontology source.'));
      return;
    }
    $msg = t('No classes or properties in the file matched your specified namespace URI. The following namespace URIs were found in the file, maybe you meant one of them?');
    $msg .= '<ul>';
    arsort($vocabulary['ignored_ns'], SORT_NUMERIC);
    foreach ($vocabulary['ignored_ns'] as $uri => $term_count) {
      $msg .= "<li>$uri ($term_count " . t('classes and properties') . ')</li>';
    }
    $msg .= '</url>';
    form_set_error('ns_uri', $msg);
    return;
  }
  
  $uris = array();
  foreach ($vocabulary['classes'] as $class) {
    $uris[] = $class['uri'];
  }
  foreach ($vocabulary['properties'] as $property) {
    if (in_array($property['uri'], $uris)) {
      form_set_error('ns_uri', 'Unsupported modelling in vocabulary file: ' . $intersection[0] . ' is typed both as a class and as a property.');
      return;
    }
  }

  // Validation successful! Store the vocabulary array in $form_state, hope this works!
  $form_state['evoc_processed_vocabulary'] = $vocabulary;
}

function evoc_add_vocabulary_form_submit($form, &$form_state) {
  // Get the vocabulary array from $form_state, should have been put there by _validate()
  if (!is_array($form_state['evoc_processed_vocabulary'])) {
    drupal_set_message('Internal error: evoc_processed_vocabulary is not present');
    return false;
  }
  $vocabulary = $form_state['evoc_processed_vocabulary'];
  
  evoc_save_vocabulary($vocabulary);
  drupal_set_message(t('External vocabulary <i>'.$vocabulary['prefix'].'</i> added. Found %classcount classes and %propertycount properties.',
      array('%classcount' => count($vocabulary['classes']), '%propertycount' => count($vocabulary['properties']))));
  $form_state['redirect'] = 'evoc';
}

/**
 * Build a confirm form for deletion of a namespace.
 * @param object $form_state
 * @param string $prefix
 * @return 
 */
function evoc_remove_confirm(&$form_state, $prefix) {
  $caption = '<p>'. t('This action cannot be undone.') .'</p>';
  $question = t('Are you sure you want to remove the %title namespace?', array('%title' => $prefix));
  $form = array('prefix' => array('#type' => 'value', '#value' => $prefix));
  return confirm_form($form, $question, 'evoc', $caption, t('Remove'), t('Cancel'));
}

/**
 * Confirmation submit callback for the remove action
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_remove_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $prefix = $form_state['values']['prefix'];
    evoc_remove_vocabulary($prefix);
    drupal_set_message(t('External vocabulary <i>'.$prefix.'</i> removed.'));
  }
  $form_state['redirect'] = 'evoc'; 
}
?>
