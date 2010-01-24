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
function evoc_admin_settings() {
	
	// get the available namespaces in the system
	$namespaces = rdf_get_namespaces();
  
  foreach ( $namespaces as $prefix => $uri ) {
  	$options[$data->id] = '';
  	$form[$prefix]['prefix'] = array('#value' => $prefix);
  	$form[$prefix]['uri_reference'] = array('#value' => $uri);

  	$operations = array();
  	$operations['edit'] = l(t('edit'), 'admin/settings/evoc/'.$prefix.'/edit', array('attributes' => array('title' => 'Edit this namespace')));
  	$operations['remove'] = l(t('remove'), 'admin/settings/evoc/'.$prefix.'/remove', array('attributes' => array('title' => 'Remove this namespace')));

  	$form[]['operations'] = array();
  	foreach ($operations as $op => $value) {
  		$form[$prefix]['operations'][$op] = array('#value' => $value);
  	}
	}
	
	/*
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save configuration'),
	);
	*/
 
	$form['back'] = array(
		'#type' => 'markup',
		'#value' => l(t('Back'), 'admin', array('attributes' => array('title' => 'Back to the admin section'))),
	);
  
	return $form;
}

/**
 * Evoc callback; Build the form that handles the edition of an namespace.
 * @param object $form_state
 * @param object $type
 * @param object $endpoint
 * @return 
 */
function evoc_admin_edit_namespace(&$form_state, $type, stdClass $namespace = NULL) {
  if( $type == 'edit' ) {
    $form['holder'] = array(
      '#type' => 'value',
      '#value' => array('prefix' => $namespace->prefix, 'uri_reference' => $namespace->uri_reference)
    );
  }

  $form['#theme'] = 'evoc_admin_edit_namespace_form';
  
  $form['prefix'] = array(
      '#type' => 'textfield',
      '#title' => t('Prefix'),
      '#default_value' => $namespace->prefix,
      '#maxsize' => 32,
      '#description' => t('Only letters, numbers, dashes, and underscores.'),
      '#required' => TRUE,
    );
    
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  $form['cancel'] = array(
		'#type' => 'markup',
		'#value' => l(t('Cancel'), 'admin/settings/evoc', array('attributes' => array('title' => 'Cancel the operation and return to the evoc admin page'))),
    '#weight' => 11,
	 );

  return $form;
}

function evoc_admin_edit_namespace_validate($form, &$form_state) {
  if (preg_match('/[^a-zA-Z0-9_\- ]/', $form_state['values']['prefix'])) {
    form_set_error('name', t('The prefix may only consist of letters, numbers, and hyphens.'));
  }
}

function evoc_admin_edit_namespace_submit($form, &$form_state) {
	$values = $form_state['values'];
	//TODO: make the update/accepted changes process
  
  $form_state['redirect'] = 'admin/settings/evoc';
}

/**
 * 
 * @param object $form
 * @return 
 */
function theme_evoc_admin_settings($form) {
	$rows = array();
	
  // Individual table headers.
	$header = array();
	$header[] = t('Prefix&nbsp;');
  $header[] = t('URI Reference&nbsp;');
  $header[] = array('data' => t('Operations'), 'colspan' => '3');
  
  foreach (element_children($form) as $key) {
		$row = array();
		if (isset($form[$key]['prefix'])) {
      $row[] = array('data' => drupal_render($form[$key]['prefix']));
			$row[] = array('data' => drupal_render($form[$key]['uri_reference']));
      
      $operations = array();
      foreach (element_children($form[$key]['operations']) as $op) {
        $operations[] = drupal_render($form[$key]['operations'][$op]);
      }

      $row = array_merge($row, $operations);
			$rows[] = $row;
		}
	}

	$output = theme('table', $header, $rows);
	//$output .= theme('pager', NULL, 5);
	
	$output .= drupal_render($form);
  
	return $output;
}

/**
 * 
 * @param unknown_type $form
 * @return unknown_type
 */
function theme_evoc_admin_edit_namespace_form($form) {
	drupal_set_title('Change the namespace prefix for '.$form['holder']['#value']['uri_reference']);
	return drupal_render($form);
}

/**
 * Build a confirm form for deletion of a namespace.
 * @param object $form_state
 * @param object $endpoint
 * @return 
 */
function evoc_admin_remove_namespace_confirm(&$form_state, $namespace) {
  $form_state['#namespace'] = $namespace;
  $caption = '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, 
    t('Are you sure you want to remove the %title namespace?', 
    array('%title' => $namespace->prefix)), 'admin/settings/evoc', 
    $caption, t('Remove'), t('Cancel'));
}

/**
 * Confirmation submit callback for the remove action
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_admin_remove_namespace_confirm_submit($form, &$form_state) {
	if ($form_state['values']['confirm']) {
		$endpoint = $form_state['#endpoint'];
    $result = db_query(db_rewrite_sql('delete from {evoc_sparql_endpoints} where id="%d"'), $endpoint->id);
    if( $result ) {			
		  evoc_set_message(t('SPARQL Endpoint has been deleted successfully.'));
    }
    else {
      evoc_set_message(t("There is some error when trying to remove range dependences."), 'warning');
    }
 	}
 
  $form_state['redirect'] = 'admin/settings/evoc'; 
}

