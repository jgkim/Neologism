<?php 
/**
 * Provide a callback for batch importing the built-in vocabularies.
 */
function evoc_import_builtin_vocabularies_form($form_state) {
  $form = array();
  
  $form['help'] = array(
      '#value' => t('To complete the initialization of this module, the built-in vocabularies have to be imported.<br/>')
  );
  
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Import built-in vocablaries',
  );
  return $form;
}

function evoc_import_builtin_vocabularies_form_submit($form, &$form_state) {
  // the parameter value 'evoc' is the page path that drupal will redirect once the batch process has been finished. 
  evoc_import_builtin_vocabularies('evoc'); 
}

function evoc_import_builtin_vocabularies($path = '') {
  // check if the variable 'evoc_missing_builtin_vocabularies' is set due to an update process
  // if it does then use it as the array of builtin vocabularies to install.
  $evoc_array_builtin_vocabularies = variable_get('evoc_missing_builtin_vocabularies', NULL);
  if (!$evoc_array_builtin_vocabularies) {
    $evoc_array_builtin_vocabularies = variable_get('evoc_builtin_vocabularies', NULL);
    if (!$evoc_array_builtin_vocabularies) {
      return;
    }
  } 
  else {
    // remove the persisten variable after use it.
    variable_del('evoc_missing_builtin_vocabularies'); 
  }
  
  $operations = array();
  foreach ($evoc_array_builtin_vocabularies as $vocab) {
    $operations[] = array(
      '_evoc_batch_import_builtin_vocabularies',
      array(
        $vocab['uri'], 
        $vocab['prefix'], 
        $vocab['filename']
      ),
    );
  }
  
  $batch = array(
    'title' => t('Downloading RDF vocabularies'),
    'finished' => '_evoc_import_batch_finished',
    'init_message' => t('The external vocabularies import process is beginning. '),
    'progress_message' => t('Imported @current of @total.'),
  	'operations' => $operations,
    'error_message' => t('The importation process encountered an error.'),
    'file' => drupal_get_path('module', 'evoc') . '/evoc.batch_import.inc',
  );
  
  batch_set($batch);
  batch_process($path);
}

/**
 * Evoc install batch callback to import external vocabularies.
 */
function _evoc_batch_import_builtin_vocabularies($uri, $prefix, $filename, &$context) {
  if (empty($context['sandbox']) ) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['max'] = 1;
    $context['message'] = t('Downloading %prefix', array('%prefix' => $prefix));
    
    module_load_include('module', 'evoc', 'evoc');
    $context['sandbox']['builtin_vocabularies_path'] = drupal_get_path('module', 'evoc').'/builtin_vocabularies';
    $context['finished'] = 0;
  }

  evoc_import_vocabulary($uri, $prefix, $context['sandbox']['builtin_vocabularies_path'].'/'.$filename);

  // Store result for post-processing in the finished callback.
  $context['results'][] = $prefix;  
    
  $context['sandbox']['progress']++;

  $context['finished'] = 1;
}

/**
 * Evoc install batch 'finished' callback.
 */
function _evoc_import_batch_finished($success, $results, $operations) {
  if ($success) {
    variable_set('evoc_builtin_vocabularies_imported', TRUE);
    
    $count = count($results);
    if ($count > 1) {
      $last_value = array_pop($results);
      $message =  t('The RDF vocabularies '.implode(', ', $results).' and '.$last_value.' have been imported successfully.');
    }
    else {
      $message = t('The RDF vocabulary '.$result[0].' has been imported.');  
    }
    
    drupal_set_message($message);
  }
  else {
    $error_operation = reset($operation);
    $operation = array_shift($error_operation);
    $arguments = array_shift($error_operation);
    $arguments_as_string = implode(', ', $arguments);
    
    watchdog('evoc', "Error when calling operation '%s'('%s')", array($operation, $arguments_as_string));
    drupal_set_message(t('An error occurred and has been recorded in the system log.'), 'error');
  }
}
