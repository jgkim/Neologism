<?php

/**
 * This file contains code for building and validating Neologism's forms for
 * creating and editing vocabularies, classes and properties.
 */

/**
 * Vocabulary form
 */
function _neologism_form_alter_vocabulary(&$form, &$form_state) {

  // Set page title and breadcrumb if we are in edit mode (node already has an ID)
  $op = $form['nid']['#value'] ? 'edit' : 'create';
  if ($op == 'edit') {
    drupal_set_title($form['#node']->field_title[0]['value']);
    drupal_set_breadcrumb(_neologism_build_breadcrumb());
  }

  $form['#validate'][] = '_neologism_form_validate_vocabulary';

  // Customize prefix field
  $form['title']['#description'] = 'Only letters, numbers, dashes, and underscores.';
  $form['title']['#size'] = 20;
  $form['title']['#field_prefix'] = url('<front>', array('absolute' => TRUE));
  $form['title']['#element_validate'][] = '_neologism_validate_id';
  
  // Add some radio buttons for choice of custom/default namespace
  $neologism_root = url('', array('absolute' => TRUE));
  $default_namespace = $neologism_root . '<span id="neologism-default-ns"><em>' . t('vocabulary-id') . '</em></span>#';
  $form['namespace']['#type'] = 'radios';
  $form['namespace']['#title'] = t('Namespace URI');
  $form['namespace']['#required'] = TRUE;
  $form['namespace']['#description'] = t('The URIs of the classes and properties in your vocabulary will start with this.');
  $form['namespace']['#options'] = array(t('Default') . ' (' . $default_namespace . ')', t('Custom') . ':');
  $form['namespace']['#weight'] = -3.5;
  $form['namespace']['#default_value'] = $form['#node']->has_custom_namespace_uri ? '1' : '0';
  // field_custom_namespace will be placed next to the Custom radio button
  // using javascript. We just hide its title here.
  $form['field_custom_namespace'][0]['#title'] = null;
  $form['field_custom_namespace']['#weight'] = -3.4; // makes it less bad if JS for some reason doesn't work

  // create a fieldset and make a copy of the field_additional_custom_rdf to locate it in a collapsable 
  // panel and unset it for the current position
  $form['aditional_custom_rdf_fieldset'] = array(
    '#type' => 'fieldset', 
    '#title' => t('Additional custom RDF'), 
    '#collapsible' => TRUE, 
    '#collapsed' => TRUE,
    '#description' => t('Here you can provide additional RDF statements that will be included in the RDF description of the vocabulary. Statements must be provided in Turtle syntax.')
  );
  $form['aditional_custom_rdf_fieldset']['field_additional_custom_rdf'] = $form['field_additional_custom_rdf'];
  unset($form['field_additional_custom_rdf']);

  // Hide the preview button
  $form['buttons']['preview']['#access'] = FALSE;
  // Hide the menu fieldset
  $form['menu']['#access'] = FALSE;
  // Hide the path fielset
  $form['path']['#access'] = FALSE;
  // Hide the layout field of the vocublary page since it's only used
  // internally by the viewer to store the diagram layout.
  $form['field_layout']['#access'] = FALSE;
}

function _neologism_form_validate_vocabulary($form, &$form_state) {

  // Validate namespace prefix
  $prefix = $form_state['values']['title'];
  // 1. Check if there is already a Neologism vocabulary with this prefix
  $count = db_result(db_query('SELECT count(*) FROM {node} WHERE nid!=%d AND title="%s" AND type = "%s"', $form_state['values']['nid'], $prefix, NEOLOGISM_CT_VOCABULARY));
  if ($count > 0) {
    form_set_error('title', t('The prefix %prefix is already in use. Please choose a different prefix.', array('%prefix' => $prefix)));
    return FALSE;
  }
  // 2. Check if some module has registered this path
  $count = db_result(db_query('SELECT COUNT(*) FROM {menu_router} WHERE path="%s"', $prefix));
  if ($count > 0) {
    form_set_error('title', t('The path %prefix is reserved by the system. Please choose a different prefix.', array('%prefix' => $prefix)));
    return FALSE;
  }
  // 3. Check if some alias has registered this path
  $src = db_result(db_query('SELECT src FROM {url_alias} WHERE dst="%s" LIMIT 1', $prefix));
  if ($src && $src != 'node/' . $form['#node']->nid) {
    form_set_error('title', t('The path %prefix is taken by other site content. Please choose a different prefix.', array('%prefix' => $prefix)));
    return FALSE;
  }
  
  // Validate custom namespace, if provided
  if ($form_state['values']['namespace'] == 1) {
    $custom_namespace = $form_state['values']['field_custom_namespace'][0]['value'];
    if (!$custom_namespace) {
      form_set_error('custom_namespace', t('You must provide a custom namespace URI, or select the default namespace.'));  
      return FALSE;
    }
    if(!rdf_is_valid_uri($custom_namespace) ) {
      form_set_error('custom_namespace', t('Invalid namespace URI. A valid example is <i>http://xmlns.com/foaf/0.1/</i>'));  
      return FALSE;
    }
    if (substr($custom_namespace, -1) != '#' && substr($custom_namespace, -1) != '/') {
      form_set_error('custom_namespace', t('The custom namespace URI must end in “#” or “/”.'));
      return FALSE;
    }
  }
}

function _neologism_form_alter_term(&$form, &$form_state, $class_or_property) {

  $parent = null;

  if (!isset($form['#node']->vocabulary)) {
    drupal_set_message('Something went wrong! No vocabulary selected!', 'error');
    return;
  }

  $current_voc = $form['#node']->vocabulary;
  $form_title = $current_voc->field_title[0]['value'];

  if (isset($form['#node']->nid)) {    // Are we editing an existing term?
    $mode = 'edit';
    $form_title = t('Editing ' . $class_or_property . ' %qname', array('%qname' => $form['#node']->qname));
    drupal_set_breadcrumb(_neologism_build_breadcrumb($current_voc->path, $current_voc->field_title[0]['value']));
  } else if (arg(3)) {    // Creating a new subclass of an existing class ...
    $mode = 'new-child';
    // Select the parent class if provided in the url of type node/1/add-class/2
    $parent = node_load(arg(3));
    drupal_set_breadcrumb(_neologism_build_breadcrumb());
  } else {    // We are creating a new term
    $mode = 'new';
    drupal_set_breadcrumb(_neologism_build_breadcrumb());
  }

  drupal_set_title($form_title);

  $form['field_vocabulary']['#default_value'][0]['nid'] = $current_voc->nid;

  // Customize prefix field
  $form['title']['#description'] = 'Only letters, numbers, dashes, and underscores.';
  $form['title']['#size'] = 20;
  $form['title']['#field_prefix'] = $current_voc->namespace_uri;
  $form['title']['#element_validate'][] = '_neologism_validate_id';
  
  $form['body_field']['body']['#rows'] = 4;
  
  // Hide unused fields and widgets
  $form['field_vocabulary']['#access'] = FALSE;
  $form['buttons']['preview']['#access'] = FALSE;
  $form['menu']['#access'] = FALSE;
  $form['path']['#access'] = FALSE;
  
  // Add a Cancel button
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#weight' => 20,
    '#attributes' => array('onclick' => 'window.location = \''.base_path().$current_voc->prefix.'\'; return false;'),
  );
  $form['buttons']['submit']['#submit'][] = '_neologism_term_form_submit';

  if ($class_or_property == 'class') {
    _neologism_form_alter_class($form, $form_state, $mode, $parent);
  } else {
    _neologism_form_alter_property($form, $form_state, $mode, $parent);
  }
}

/**
 * Set up widgets specific to classes
 */
function _neologism_form_alter_class(&$form, &$form_state, $mode, &$parent) {

  if ($mode == 'edit') {
    // Tell the widget that we are editing a term
    $form['field_superclass2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;
    $form['field_disjointwith2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;
  } else if ($mode == 'new-child') {
    // extra_values is an optional parameter for #mxcbs_behaviour attribute that contain extra default values for the widget tree
    $form['field_superclass2']['#mxcbs_behaviour']['extra_values'] = array($parent->qname);
    $form['field_superclass2']['#mxcbs_behaviour']['execute_preprocess'] = TRUE;
  }
  
  $form['field_superclass2']['#mxcbs_behaviour']['owntitle'] = FALSE;
  $form['field_superclass2']['#mxcbs_behaviour']['handler'] = 'Neologism.createSuperclassSelecctionWidget';
  $form['field_superclass2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/fulltree';

  $form['field_disjointwith2']['#mxcbs_behaviour']['owntitle'] = FALSE;
  $form['field_disjointwith2']['#mxcbs_behaviour']['handler'] = 'Neologism.createDisjointwithSelecctionWidget'; 
  $form['field_disjointwith2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/fulltree'; 

  $form['#validate'][] = '_neologism_form_validate_class';
}

/**
 * Form validation handler for the neo_class node form.
 *
 * @param object $form
 * @param object $form_state
 * @return 
 */
function _neologism_form_validate_class($form, &$form_state) {
    if(!empty($form_state['values']['field_superclass2']) ) {
      // TODO do a global select and then compare with the selected value. 
      // This way consume too much resources whether there exist many classes.
      foreach( $form_state['values']['field_superclass2'] as $value ) {
        // FIXME there is some error with external superclasses that doesn't has superclasses
        if( !evoc_term_exists(NULL, $value['evoc_term'], TRUE) ) {
            form_set_error('field_superclass2', t('You must select a super class from one of the class that match your criterial. If that class does not exists in the system, please import it.'));
        }
      }
    }
}

function _neologism_form_alter_property(&$form, &$form_state, $mode, &$parent) {
  
  // add behaviours for evoc fields if they are using the MXCheckBox selection widget
  $form['field_domain2']['#mxcbs_behaviour']['owntitle'] = FALSE;
  $form['field_domain2']['#mxcbs_behaviour']['handler'] = 'Neologism.createDomainSelecctionWidget';
  $form['field_domain2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/fulltree';
  
  $form['field_range2']['#mxcbs_behaviour']['owntitle'] = TRUE;
  $form['field_range2']['#mxcbs_behaviour']['handler'] = 'Neologism.createRangeSelecctionWidget';
  $form['field_range2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/fulltree';
  $form['field_range2']['#prefix'] = '<div id="range-treeview">';
  $form['field_range2']['#suffix'] = '</div>';
  
  // add behaviours for evoc fields if they are using the MXCheckBox selection widget
  $form['field_superproperty2']['#mxcbs_behaviour']['owntitle'] = FALSE;
  $form['field_superproperty2']['#mxcbs_behaviour']['handler'] = 'Neologism.createSuperpropertySelecctionWidget';
  $form['field_superproperty2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/propertiesfulltree';
  
  // add behaviours for field_inverse2 field
  $form['field_inverse2']['#mxcbs_behaviour']['owntitle'] = FALSE;
  $form['field_inverse2']['#mxcbs_behaviour']['handler'] = 'Neologism.createInverseSelecctionWidget';
  $form['field_inverse2']['#mxcbs_behaviour']['url_gateway'] = 'neologism/json/propertiesfulltree';
  $form['field_inverse2']['#prefix'] = '<div id="inverse-treeview">';
  $form['field_inverse2']['#suffix'] = '</div>';
  
  // add the group for ranges
  $form['range_group']['#weight'] = $form['field_range2']['#weight'] - 0.5;
  $form['range_group']['resource_type'] = array(
    '#type' => 'radios',
    '#title' => t('Range'),
    '#options' => array(
      1 => t('The value of this property is another resource.'),
      2 => t('The value of this property is a literal (string, number, date, ...).'),
      3 => t('The value of this property can be either.')
    ),
    //'#description' => t('Description should goes here....'),
    '#default_value' => 1,
    '#prefix' => '<div id="range-group-resourcetype">',
    '#suffix' => '</div>',
    '#attributes' => array('onClick' => 'Neologism.checkResourceType()'),
  );
  
  $form['range_group']['data_types'] = array(
    '#title' => '',//t('Data Types'),
    '#type' => 'select',
    '#description' => t('Select the datatype for this property.'),
    '#options' => neologism_get_literal_datatype_list(),
    '#multiple' => FALSE,
    '#prefix' => '<div id="range-group-datatypes">',
    '#suffix' => '</div>'
  );
 
  if ($mode == 'edit') {
    // why need to inform to the widget that there is comming an edition for a value, so the widget can
    // handle the situation.
    $form['field_domain2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;
    $form['field_range2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;
    $form['field_superproperty2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;
    $form['field_inverse2']['#mxcbs_behaviour']['editing_value'] = $form['#node']->qname;

    // set the default value depending of the current property range value
    $range = $form['#node']->field_range2[0]['evoc_term'];
    if( isset($range) && _neologism_is_literal_datatype($range) ) {
      $form['range_group']['data_types']['#default_value'] = $range;
      // set the radio button to The value of this property is a literal (string, number, date, ...).
      $form['range_group']['resource_type']['#default_value'] = 2;
    }
  } else if ($mode == 'new-child') {
    // extra_values is an optional parameter for #mxcbs_behaviour attribute that contain extra default values for the widget tree
    $form['field_superproperty2']['#mxcbs_behaviour']['extra_values'] = array($parent->qname);
    $form['field_superproperty2']['#mxcbs_behaviour']['execute_preprocess'] = TRUE;
  }

  $form['#validate'][] = '_neologism_form_validate_property';
}

/**
 * Handler for validation form for the neo_property node form
 * 
 * @param object $form
 * @param object $form_state
 * @return 
 */
function _neologism_form_validate_property($form, &$form_state) {
  // check for a new property
  //if( !$form_state['values']['nid'] ) {
    $vocabulary_nid = $form_state['values']['field_vocabulary'][0]['nid'];
    $property_title = $form_state['values']['title'];
    
    // check for the property's domain. if the class selected already exists in the system
    foreach( $form_state['values']['field_domain2'] as $value ) {
      if( !evoc_term_exists(NULL, $value['evoc_term'], TRUE) ) {
        form_set_error('field_domain2', t('You must select a domain from one of the class that match your criterial. If that class does not exists in the system, please import it.'));
      }
    }
    
    // if $form_state['values']['resource_type'] == '1' everithing come fine
    if( $form_state['values']['resource_type'] == '2' ) {
      $form_state['values']['field_range2'][0]['evoc_term'] = $form_state['values']['data_types'];
      $form_state['values']['field_inverse2'] = array();
    }
    else if( $form_state['values']['resource_type'] == '3' ) {
      $form_state['values']['field_range2'][0] = NULL;
    }
    
    // Comment by Guido Cecilio 2nd Febraury 2010
    // we check at validation time if there is no domain or range and add by default rdfs:Resource (DEFAULT_RDFS_SUPERCLASS)
    if( empty($form_state['values']['field_domain2']) ) {
        $form_state['values']['field_domain2'][0]['evoc_term'] = DEFAULT_RDFS_SUPERCLASS;
    }
    if( empty($form_state['values']['field_range2']) ) {
        $form_state['values']['field_range2'][0]['evoc_term'] = DEFAULT_RDFS_SUPERCLASS;
    }

    /*
    if( $form_state['values']['field_literal_as_range'][0]['value'] ) {
      $range = "rdfs:Literal";
      // this would be used when save the property to the repository
      $form_state['values']['field_range2'][0]['evoc_term'] = $range;
    }
    */
    
    /*
     * We don't need to check something that is filtered for the system. So we need to remove this 
     * commented piece of code afeter be sure of that
     * 
    // check for the property's ranges. if the range already exists in the system
    if( !empty($form_state['values']['field_range2']) ) {
      foreach( $form_state['values']['field_range2'] as $value ) {
        if( !evoc_term_exists(NULL, $value['evoc_term'], TRUE) ) {
          form_set_error('field_range2', t('You must select a range from one of the terms that match your criterial. If that class does not exists in the system, please import it.'));
        }
      }
    }
    */
    
    // check for the property's superproperty field. 
    foreach( $form_state['values']['field_superproperty2'] as $value ) {
      if( !evoc_term_exists(NULL, $value['evoc_term'], FALSE) ) {
        form_set_error('field_superproperty2', t('There is some error selecting your superproperty, the term does not exists in the system.'));
      }
    }
  //}
}

/**
 * Callback for submission of the class/property forms. Redirects to the vocabulary.
 */
function _neologism_term_form_submit($form, &$form_state) {
  $form_state['redirect'] = 'node/'.$form_state['values']['field_vocabulary'][0]['nid'];
}

/**
 * Custom form validation function for IDs (prefixes, class names, property names).
 * To be used with $form['my_element']['#element_validate'].
 */
function _neologism_validate_id($element, &$form_state) {
  if (!_neologism_is_valid_id($element['#value'])) {
    form_set_error($element,
        t("%id is not a valid identifier. It should start with a letter, and should contain only letters, numbers, dashes, and underscores.",
            array('%id' => $element['#value'])));
  }
}
