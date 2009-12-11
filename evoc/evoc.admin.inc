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
  $query = 'select * from {evoc_sparql_endpoints}';
	$rs = db_query($query);
 
	$default = variable_get('evoc_sparqlendpoint', 1);
  $enabledEndpoints = array();
  $disabled = array();
 
	if ($rs) {
		while ($data = db_fetch_object($rs)) {
      $options[$data->id] = '';
      $form[$data->id]['name'] = array('#value' => stripslashes($data->name));
      //$form[$data->id]['enabled'] = array('#value' => stripslashes($data->enabled));
      $form[$data->id]['status'] = array('#value' => stripslashes($data->status));
      $form[$data->id]['endpoint'] = array('#value' => stripslashes($data->endpoint));
      //$form[$data->id]['webform'] = array('#value' => stripslashes($data->webform));
      $form[$data->id]['comment'] = array('#value' => stripslashes($data->comment));
      //$form[$data->id]['data_exposed'] = array('#value' => stripslashes($data->data_exposed));
      
      if( $data->enabled ) {
        $enabledEndpoints[] = $data->id;  
      }
      else {
        $disabled[] = $data->id;
      }

      $operations = array();
      $operations['edit'] = l(t('edit'), 'admin/settings/evoc/'. $data->id .'/edit', array('attributes' => array('title' => 'Edit this endpoint.')));
      $operations['delete'] = l(t('delete'), 'admin/settings/evoc/'. $data->id .'/delete', array('attributes' => array('title' => 'Delete this endpoint.')));
      
      $form[]['operations'] = array();
      foreach ($operations as $op => $value) {
        $form[$data->id]['operations'][$op] = array('#value' => $value);
      }
		}
	}
	
  $form['default'] = array(
		'#type' => 'radios',
		'#options' => $options,
		'#default_value' => $default,
	);
  
  $form['enabled'] = array(
		'#type' => 'checkboxes',
		'#options' => $options,
		'#default_value' => $enabledEndpoints,
    //'#disabled' => $disabled,
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save configuration'),
	);
 
	$form['back'] = array(
		'#type' => 'markup',
		'#value' => l(t('Back'), 'admin', array('attributes' => array('title' => 'Back to admin section.'))),
	);
  
	return $form;
}

/**
 * Submit callback
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_admin_settings_submit($form, &$form_state) {
  $endpoint = $form_state['values'];
  
  foreach ( $endpoint['enabled'] as $key => $value ) {
    db_query("update {evoc_sparql_endpoints} set enabled = '%d' where id = '%d'", ($value != 0), $key);
  }
  
  if( !$endpoint['enabled'][$endpoint['default']] ) {
    foreach ( $endpoint['enabled'] as $key => $value ) {
      if( $value ) {
        $endpoint['default'] = $key;
        break;
      }
    }    
  }
  
  variable_set('evoc_sparqlendpoint', $endpoint['default']);
  
  $form_state['redirect'] = 'admin/settings/evoc';
}

/**
 * Evoc callback; Build the form that handles the adding/editing of an endpoint.
 * @param object $form_state
 * @param object $type
 * @param object $endpoint
 * @return 
 */
function evoc_edit_sparql_endpoint(&$form_state, $type, stdClass $endpoint = NULL) {
  if( $type == 'edit' ) {
    $form['#insert'] = FALSE;
    
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('evoc_delete_endpoint_submit'),
      '#weight' => 10,
    );
    
    $form['holder'] = array(
      '#type' => 'value',
      '#value' => array('id' => $endpoint->id)
    );
  }
  else {
    $endpoint = new stdClass();
    $endpoint->name = '';
    $endpoint->enabled = FALSE;
    $endpoint->status = '';
    $endpoint->endpoint = '';
    $endpoint->webform = '';
    $endpoint->comment = '';
    $endpoint->data_exposed = ''; 
    $form['#insert'] = TRUE;
  }
  $form['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name/Project'),
      '#default_value' => $endpoint->name,
      '#maxsize' => 255,
      '#description' => t('The endpoint name or project.'),
      '#required' => TRUE,
    );
    
  $form['status'] = array(
    '#type' => 'textfield',
    '#title' => t('Status'),
    '#default_value' => $endpoint->status,
    '#maxsize' => 255,
    '#description' => t('Status defined by the endpoint\'s status. Just write alive in the textfield to use it as a valid endpoint.'),
    '#required' => TRUE,
  );
  
  $form['endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Sparql endpoint'),
    '#maxsize' => 255,
    '#description' => t('The SPARQL Endpoint link.'),
    '#default_value' => $endpoint->endpoint,
    '#required' => TRUE,
  );
  
  $form['webform'] = array(
    '#type' => 'textfield',
    '#title' => t('Webform'),
    '#maxsize' => 255,
    '#description' => t(''),
    '#default_value' => $endpoint->webform,
  );
  
  $form['comment'] = array(
    '#type' => 'textarea',
    '#title' => t('Comment'),
    '#description' => t(''),
    '#default_value' => $endpoint->comment,
  );
  
  $form['data_exposed'] = array(
    '#type' => 'textarea',
    '#title' => t('Data exposed'),
    '#default_value' => $endpoint->data_exposed,
  );
  
  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled endpoint.'),
    '#description' => t('Check on this checkbox to enabled this sparql endpoint to query in the cloud.'),
    '#default_value' => $endpoint->enabled,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  $form['cancel'] = array(
		'#type' => 'markup',
		'#value' => l(t('Cancel'), 'admin/settings/evoc'),
    '#weight' => 11,
	 );

  return $form;
}

/**
 * Validates the human and machine-readable names when adding
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_edit_sparql_endpoint_validate($form, &$form_state) {
  //TODO this function need some tune
  $item = $form_state['values'];
  
  if (preg_match('/[^a-zA-Z0-9_\- ]/', $item['name'])) {
    form_set_error('name', t('The name may only consist of letters, numbers, and hyphens.'));
  }
  
  if (preg_match('/[^a-zA-Z0-9_\- ]/', $item['name'])) {
    form_set_error('name', t('The name may only consist of letters, numbers, and hyphens.'));
  }
  
  if ( !preg_match("/^[a-zA-Z]+[:\/\/]+[A-Za-z0-9\-_]+\\.+[A-Za-z0-9\.\/%&=\?\-_]+$/i", $item['endpoint']) ) {
    form_set_error('endpoint', t('Invalid endpoint. Please provide a valid sparql endpoint.'));
  }
  
  if (strlen($item['name']) > 255) {
    form_set_error('name', format_plural(255, "The name can't be longer than 1 character.", "The name can't be longer than @count characters."));
  }
  
  if ($form['#insert']) {
    if ( db_result(db_query("select endpoint from {evoc_sparql_endpoints} where endpoint = '%s'", $item['endpoint'])) ) {
      form_set_error('name', t('There is already Name for this sparql endpoint. Plase check the endpoint field.'));
    }
  }
  
  if( $item['op'] == 'Delete' ) {
    form_set_value($form['holder'], $item['holder'], $form_state);
  }
  
}

/**
 * Submit function for adding or editing a endpoints.
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_edit_sparql_endpoint_submit($form, &$form_state) {
  $endpoint = $form_state['values'];
  if ($form['#insert']) {
    db_query("insert into {evoc_sparql_endpoints} (name, enabled, status, endpoint, webform, comment, data_exposed) values ('%s', '%d', '%s', '%s', '%s', '%s', '%s')", 
      $endpoint['name'], $endpoint['enabled'], $endpoint['status'], $endpoint['endpoint'], $endpoint['webform'], $endpoint['comment'], $endpoint['data_exposed']);
  }
  else {
    db_query("update {evoc_sparql_endpoints} set name = '%s', enabled = '%d', status = '%s', endpoint = '%s', webform = '%s', comment = '%s', data_exposed = '%s'  
      where id = '%d'", $endpoint['name'], $endpoint['enabled'], $endpoint['status'], $endpoint['endpoint'], $endpoint['webform'], $endpoint['comment'], $endpoint['data_exposed'], $endpoint['holder']['id']);
  }
  
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
	$header[] = array('data' => t('Default&nbsp;'), 'class' => 'radio');
	$header[] = t('Name&nbsp;');
	$header[] = array('data' => t('Enabled&nbsp;'), 'class' => 'checkbox');
	$header[] = t('Status&nbsp;');
  $header[] = t('Endpoint&nbsp;');
  $header[] = t('Comment&nbsp;');
  $header[] = array('data' => t('Operations'), 'colspan' => '3');
  
  foreach (element_children($form) as $key) {
		$row = array();
		if (isset($form[$key]['name'])) {
      if( $form['enabled'][$key]['#default_value'] == FALSE ) {
        $form['default'][$key]['#attributes'] = array('disabled' => 'disabled');  
      }
      $row[] = array('data' => drupal_render($form['default'][$key]));
			$row[] = array('data' => drupal_render($form[$key]['name']));
      $row[] = array('data' => drupal_render($form['enabled'][$key]));
      $row[] = array('data' => drupal_render($form[$key]['status']));
      $row[] = array('data' => drupal_render($form[$key]['endpoint']));
			$row[] = array('data' => drupal_render($form[$key]['comment']));
      
      $operations = array();
      foreach (element_children($form[$key]['operations']) as $op) {
        $operations[] = drupal_render($form[$key]['operations'][$op]);
      }

      $row = array_merge($row, $operations);
			$rows[] = $row;
		}
	}

	$output = theme('table', $header, $rows);
	$output .= drupal_render($form);
  
	return $output;
}

/**
 * Submit callback for delection of a sparql endpoint
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_delete_endpoint_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/settings/evoc/'. $form_state['values']['holder']['id'] .'/delete';
}

/**
 * Build a confirm form for deletion of a sparql endpoint.
 * @param object $form_state
 * @param object $endpoint
 * @return 
 */
function evoc_delete_endpoint_confirm(&$form_state, $endpoint) {
  $form_state['#endpoint'] = $endpoint;
  $caption = '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, 
    t('Are you sure you want to delete this sparql endpoint %title?', 
    array('%title' => $endpoint->name)), 'admin/settings/evoc', 
    $caption, t('Delete'), t('Cancel'));
}

/**
 * Confirmation submit callback
 * @param object $form
 * @param object $form_state
 * @return 
 */
function evoc_delete_endpoint_confirm_submit($form, &$form_state) {
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