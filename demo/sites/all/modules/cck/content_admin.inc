<?php
// $Id: content_admin.inc,v 1.28.2.38 2007/08/09 18:58:52 yched Exp $

/**
 * @file
 * Administrative interface for content type creation.
 */

/**
 * Menu callback; lists all defined fields for quick reference.
 */
function _content_admin_type_fields() {
  $fields = content_fields();

  $header = array(t('Field name'), t('Field type'), t('Used in'));
  $rows = array();
  foreach ($fields as $field) {
    $row = array();
    $row[] = $field['field_name'];
    $row[] = $field['type'];

    $types = array();
    $result = db_query("SELECT nt.name, nt.type FROM {node_field_instance} nfi LEFT JOIN {node_type} nt ON nt.type = nfi.type_name WHERE nfi.field_name = '%s' ORDER BY nt.name ASC", $field['field_name']);
    while ($type = db_fetch_array($result)) {
      $content_type = content_types($type['type']);
      $types[] = l($type['name'], 'admin/content/types/'. $content_type['url_str'] .'/fields');
    }
    $row[] = implode(', ', $types);

    $rows[] = $row;
  }

  $output = theme('table', $header, $rows);

  return $output;
}

/**
 * Menu callback; presents a listing of fields for a content type.
 *
 * Form includes form widgets to set weight and group for each item
 * and displays other form elements and their weights to make it
 * easier to place CCK fields in the form and see where they will appear.
 */
function content_admin_field_overview_form($type_name) {
  $form = array();

  $type = content_types($type_name);
  $field_types = _content_field_types();

  // Create a dummy node and form and call hook_form_alter()
  // to produce an array of fields and weights added to the node by all modules.
  $dummy_node = new stdClass();
  $dummy_node->type = $type['type'];

  // Some modules (userreview...) "hide" their node forms, resulting in no field
  // being listed. We set a special flag to inform them this form is special.
  $dummy_node->cck_dummy_node_form = TRUE;

  $dummy_form_id = $type['type'] .'_node_form';
  $dummy_form = node_form($dummy_node);
  foreach (module_implements('form_alter') as $module) {
    $function = $module .'_form_alter';
    $function($dummy_form_id, $dummy_form);
  }

  // Move group fields into a 'fields' subgroup to make them easier to identify.
  // Remove fields that are used in groups from the form, the group will handle them.
  if (module_exists('fieldgroup')) {
    $form['#groups']       = fieldgroup_groups($type['type']);
    $form['#group_labels'] = _fieldgroup_groups_label($type['type']);
    if (!$form['#groups']) {
      drupal_set_message(t('There are no groups configured for this content type.'));
    }
    foreach ($form['#groups'] as $group) {
      foreach ($group['fields'] as $field_name => $field) {
        unset($dummy_form[$field_name]);
      }
    }
  }

  if (!$type['fields']) {
    drupal_set_message(t('There are no fields configured for this content type.'));
  }

  if (!$type['fields'] && !$form['#groups']) {
    return $form;
  }

  $form['disabled']['#value'] = array();

  // Iterate through the dummy form and add top-level fields and weights to a table.
  // Construct the table values in an array '#table' that FAPI will ignore, keyed on the item's weight.
  // Create separate form elements for each weight and group value and put a placeholder for each in #table.

  foreach ($dummy_form as $key => $value) {

    // Limiting weight to < 10 will keep workflow and submit elements from being added to the overview table.
    // They're outside the weight range allowed for CCK fields, so won't interfere with field placement.

    if (is_array($value) && (isset($value['#weight']) || $key == 'body_filter') && $value['#weight'] <= 10) {

      // if this item is a group, insert group info into table, then add all the group fields below it
      if (substr($key, 0, 6) == 'group_' && isset($form['#groups'])) {
        $row = $group_form = array();
        $row['label']     = $form['#group_labels'][$form['#groups'][$key]['group_name']];
        $row['name']      = $form['#groups'][$key]['group_name'];
        $row['type']      = t('group');
        $row['weights']   = 'form-group-weights';
        $row['groups']    = '';
        $row['configure'] = l(t('configure'), 'admin/content/types/'. $type['url_str'] .'/groups/'. $form['#groups'][$key]['group_name'] .'/edit');
        $row['remove']    = l(t('remove'), 'admin/content/types/'. $type['url_str'] .'/groups/'. $form['#groups'][$key]['group_name'] .'/remove');
        $data = $row;

        $form['group-weights'][$key] = array('#type' => 'weight', '#default_value' => $value['#weight']);
        foreach ($form['#groups'][$key]['fields'] as $field_name => $field) {
          $row = array();
          $field = $type['fields'][$field_name];
          $row['label']     = $field['widget']['label'];
          $row['name']      = $field['field_name'];
          $row['type']      = $field_types[$field['type']]['label'];
          $row['weights']   = 'form-field-weights';
          $row['groups']    = 'form-field-groups';
          $row['configure'] = l(t('configure'), 'admin/content/types/'. $type['url_str'] .'/fields/'. $field_name);
          $row['remove']    = l(t('remove'), 'admin/content/types/'. $type['url_str'] .'/fields/'. $field_name .'/remove');
          $group_form[$field['widget']['weight']][] = array($field_name => $row);

          $form['field-weights'][$field_name] = array('#type' => 'weight', '#default_value' => $field['widget']['weight']);
          $form['field-groups'][$field_name]  = array('#type' => 'select', '#options' => $form['#group_labels'], '#default_value' => fieldgroup_get_group($type['type'], $field_name));
          $form['field-groups-defaults'][$field_name]  = array('#type' => 'hidden', '#value' => fieldgroup_get_group($type['type'], $field_name));
        }
        // sort the group fields by weight
        ksort($group_form);
        $group = (array) $data + array('fields' => $group_form);
        $form['#table'][$value['#weight']][] = array($key => $group);
      }

      // else if this item is a top-level field, insert field row into the table
      elseif (substr($key, 0, 6) == 'field_') {
        $row = array();
        $field = $type['fields'][$key];
        $row['label']     = $field['widget']['label'];
        $row['name']      = $field['field_name'];
        $row['type']      = $field_types[$field['type']]['label'];
        $row['weights']   = 'form-field-weights';
        if (isset($form['#groups'])) {
          $row['groups']  = 'form-field-groups';
        }
        $row['configure'] = l(t('configure'), 'admin/content/types/'. $type['url_str'] .'/fields/'. $key);
        $row['remove']    = l(t('remove'), 'admin/content/types/'. $type['url_str'] .'/fields/'. $key .'/remove');
        $form['#table'][$field['widget']['weight']][] = array($key => $row);

        $form['field-weights'][$key] = array('#type' => 'weight', '#default_value' => $field['widget']['weight']);
        if (isset($form['#groups'])) {
          $form['field-groups'][$key]  = array('#type' => 'select', '#options' => $form['#group_labels'], '#default_value' => fieldgroup_get_group($type['type'], $key));
        }
      }

      // otherwise this is some other form field or fieldset
      // if it has a weight display it as a disabled item
      else {
        $row = array();
        $row['label']     = $key == 'body_filter' ? t('body') : $key;
        $row['name']      = $key;
        $row['type']      = $key;
        $row['weights']   = 'form-field-weights';
        if (isset($form['#groups'])) {
          $row['groups']  = '';
        }
        $row['configure'] = '';
        $row['remove']    = '';
        $form['#table'][$value['#weight']][] = array($key => $row);
        $form['disabled']['#value'][] = $key;

        $form['field-weights'][$key] = array('#type' => 'weight', '#default_value' => $value['#weight'], '#disabled' => TRUE);
      }
    }
  }

  // sort the table by weight
  ksort($form['#table']);

  // add submit buttons and hidden fields
  $form['submit']                         = array('#type' => 'submit', '#value' => t('Update'));
  $form['field-weights']['#tree']         = TRUE;
  $form['group-weights']['#tree']         = TRUE;
  $form['field-groups']['#tree']          = TRUE;
  $form['field-groups-defaults']['#tree'] = TRUE;
  $form['disabled']['#type']              = 'hidden';
  $form['disabled']['#value']             = serialize($form['disabled']['#value']);
  $form['type_name']['#type']             = 'hidden';
  $form['type_name']['#value']            = $type['type'];

  return $form;
}

/**
 *  Theme the field overview table by iterating through the form and rendering form elements in table cells
 */
function theme_content_admin_field_overview_form($form) {
  if (!$form['#table']) {
    return;
  }

  // The css for this form contains non-validating styles,
  // so we use a separate file, included only on the relevant page.
  drupal_add_css(drupal_get_path('module', 'content') .'/content_admin.css');

  $disabled = unserialize($form['disabled']['#value']);

  if (module_exists('fieldgroup')) {
    $header = array(t('Label'), t('Name'), t('Type'), t('Weight'), t('Group'), array('data' => t('Operations'), 'colspan' => 2));
    $colspan = 7;
  }
  else {
    $header = array(t('Label'), t('Name'), t('Type'), t('Weight'), array('data' => t('Operations'), 'colspan' => 2));
    $colspan = 6;
  }

  $rows = array();
  $i = 0;

  // The table was created in the form
  // iterate through it and render form elements when placeholders are encountered
  // then run the rows array through theme_table().
  foreach ($form['#table'] as $weight => $frow) {
    foreach ($frow as $delta => $item) {
      foreach ($item as $fname => $field) {
        $row = array();
        $class = 'content-field-overview-enabled';
        if (in_array($fname, $disabled)) {
          $class = 'content-field-overview-disabled';
        }
        foreach ($field as $col => $cell) {
          // display cols other than the group 'fields' col
          if ($col != 'fields') {
            switch ($cell) {
            case 'form-field-weights':
              $row[] = drupal_render($form['field-weights'][$fname]);
              break;
            case 'form-group-weights':
              $row[] = drupal_render($form['group-weights'][$fname]);
              break;
            case 'form-field-groups':
              $row[] = drupal_render($form['field-groups'][$fname]);
              break;
            default:
              $row[] = array('data' => $cell, 'class' => $class);
            }
          }

          elseif (isset($form['#groups'])) {
            // if this form contains groups info and this is a group 'fields' col, finish the previous row
            // then theme the 'fields' col with a fieldset containing a table and the group fields
            $grows = array();

            if (!empty($cell)) {
              foreach ($cell as $gweight => $grow) {
                foreach ($grow as $gdelta => $gitem) {
                  foreach ($gitem as $gname => $gfield) {
                    $grow = array();
                    foreach ($gfield as $gcol => $gcell) {
                      switch ($gcell) {
                        case 'form-field-weights':
                          $grow[] = drupal_render($form['field-weights'][$gname]);
                          break;
                        case 'form-field-groups':
                          $grow[] = drupal_render($form['field-groups'][$gname]);
                          break;
                        default:
                          $grow[] = $gcell;
                      }
                    }
                    $grows[] = array('data' => $grow, 'class' => 'content-field-overview-enabled');
                  }
                }
              }
            }
            else {
              $grows[] = array(array('data' => t('No fields have been added to this group.'), 'colspan' => $colspan, 'class' => 'content-field-overview-empty'));
            }

            // add the group row in its own table above the group fields table, then reset $row().
            $fieldset = array(
              '#title' => t('!label (!name)', array('!label' => $form['#group_labels'][$fname], '!name' => $fname)),
              '#collapsible' => TRUE,
              '#collapsed' => FALSE,
              '#value' => theme('table', array(), array(array('data' => $row, 'class' => 'content-field-overview-group'))) . theme('table', $header, $grows),
              );
            $row = array();
            $row[] = array(
              'data' => theme('fieldset', $fieldset),
              'colspan' => $colspan,
              'class' => 'active',
              );
            $grows = array();
          }
        }
        $rows[] = $row;
      }
    }
  }
  $output  = theme('table', $header, $rows, array('class' => 'content-field-overview'));
  $output .= drupal_render($form);
  return $output;

}

function content_admin_field_overview_form_submit($form_id, $form_values) {
  $msg = FALSE;
  foreach ((array) $form_values['field-groups'] as $key => $value) {
    if ($key && !in_array($key, unserialize($form_values['disabled']))) {
      $values  = array('field_name' => $key, 'group' => $value, 'type_name' => $form_values['type_name']);
      $default = $form_values['field-groups-defaults'][$key];
      fieldgroup_content_admin_form_submit('_content_admin_field', $values, $default);
      $msg = TRUE;
    }
  }
  if ($msg) {
    drupal_set_message(t('Updated field groups.'));
  }
  $msg = FALSE;
  foreach ((array) $form_values['group-weights'] as $key => $value) {
    if ($key && !in_array($key, unserialize($form_values['disabled']))) {
      db_query("UPDATE {node_group} SET weight = %d WHERE type_name = '%s' AND group_name = '%s'",
        $value, $form_values['type_name'], $key);
      $msg = TRUE;
    }
  }
  if ($msg) {
    drupal_set_message(t('Updated group weights.'));
  }
  $msg = FALSE;
  foreach ((array) $form_values['field-weights'] as $key => $value) {
    if ($key && !in_array($key, unserialize($form_values['disabled']))) {
      db_query("UPDATE {node_field_instance} SET weight = %d WHERE type_name = '%s' AND field_name = '%s'",
        $value, $form_values['type_name'], $key);
      $msg = TRUE;
    }
  }
  if ($msg) {
    drupal_set_message(t('Updated field weights.'));
  }
  content_clear_type_cache();
  cache_clear_all('fieldgroup_data', 'cache_content');

}

/**
 * Menu callback; presents a listing of fields display settings for a content type.
 *
 * Form includes form widgets to select which fields appear for teaser, full node...
 * and how the field labels should be rendered
 */
function content_admin_display_overview_form($type_name) {
  $type = content_types($type_name);
  $field_types = _content_field_types();

  $form = array();
  $form['type_name'] = array('#type' => 'hidden', '#value' => $type['type']);

  if (empty($type['fields'])) {
    drupal_set_message(t('There are no fields configured for this content type.'));
    return $form;
  }
  $form['#tree'] = TRUE;
  foreach ($type['fields'] as $field) {
    $form['fields'][$field['field_name']] = _content_admin_display_overview_row($field, $field_types[$field['type']]);
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'), '#weight' => 10);
  return $form;
}

function theme_content_admin_display_overview_form($form) {
  $header = array(t('Field'), t('Type'), t('Label'));
  foreach (_content_admin_display_contexts() as $key => $title) {
    $header[] = $title;
  }
  $rows = array();
  foreach (element_children($form['fields']) as $field) {
    $row = array();
    foreach (element_children($form['fields'][$field]) as $key) {
      $row[] = drupal_render($form['fields'][$field][$key]);
    }
    $rows[] = $row;
  }
  $output = '';
  if (!empty($rows)) {
    $output  = theme('table', $header, $rows, array('class' => 'content-field-display-overview'));
  }
  $output .= drupal_render($form);
  return $output;
}

function content_admin_display_overview_form_submit($form_id, $form_values) {
  $type = $form_values['type_name'];

  if (isset($form_values['fields'])) {
    foreach ($form_values['fields'] as $fieldname => $fieldvalues) {
      $display_settings = array();
      foreach ($fieldvalues as $key => $value) {
        $display_settings[$key] = $value;
      }

      db_query("UPDATE {node_field_instance} SET display_settings = '%s' WHERE type_name = '%s' AND field_name = '%s'",
        serialize($display_settings), $type, $fieldname);
    }
    content_clear_type_cache();
  }
  drupal_set_message(t('Your settings have been saved.'));
}

function _content_admin_display_overview_row($field, $field_type) {
  $defaults = $field['display_settings'];

  $options = array();
  foreach ($field_type['formatters'] as $name => $formatter_info) {
    $options[$name] = $formatter_info['label'];
  }
  $options['hidden'] = t('<Hidden>');

  $label_options = array(
    'above' => t('Above'),
    'inline' => t('Inline'),
    'hidden' => t('<Hidden>'),
  );

  $row = array();
  $row['type_label']  = array('#value' => $field['widget']['label']);
  $row['type']        = array('#value' => $field_type['label']);

  $row['label']['format'] = array(
    '#type' => 'select',
    '#options' => $label_options,
    '#default_value' => isset($defaults['label']['format']) ? $defaults['label']['format'] : 'above',
  );

  foreach (_content_admin_display_contexts() as $key => $title) {
    $row[$key]['format'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => isset($defaults[$key]['format']) ? $defaults[$key]['format'] : 'default',
    );
  }
  return $row;
}

function _content_admin_display_contexts() {
  return array(
    'teaser' => t('Teaser'),
    'full' => t('Full'),
  );
}

/**
 * Menu callback; presents the form for adding a new field.
 */
function _content_admin_field_add($type_name) {
  // make sure the old field list gets cleared before creating the new one
  if (!isset($_POST['edit'])) {
    content_clear_type_cache();
  }
  $output = drupal_get_form('_content_admin_field_add_existing', $type_name);
  $output .= drupal_get_form('_content_admin_field_add_new', $type_name);
  return $output;
}

function _content_admin_field_add_existing($type_name) {
  $output = '';
  $type = content_types($type_name);
  $fields = content_fields();
  $form = array();

  $options = array();
  foreach ($fields as $field) {
    if (!isset($type['fields'][$field['field_name']]))
    $options[$field['field_name']] = t($field['widget']['label']) .' ('. $field['field_name'] .')';
  }
  if ($options) {
    $form['existing'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add existing field'),
    );
    $form['existing']['field_name'] = array(
      '#type' => 'select',
      '#required' => TRUE,
      '#options' => $options,
    );
    $form['existing']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add field'),
    );
    $form['existing']['type_name'] = array(
      '#type' => 'value',
      '#value' => $type_name,
    );
  }
  return $form;
}

function _content_admin_field_add_new($type_name, $new_field_name = '') {
  $field_types = _content_field_types();
  $widget_types = _content_widget_types();
  $form = array();

  $field_type_options = array();
  foreach ($field_types as $field_name => $field_type) {
    foreach ($widget_types as $widget_name => $widget_type) {
      if (in_array($field_name, $widget_type['field types'])) {
        $field_type_options[$field_name .'-'. $widget_name] = $widget_type['label'];
      }
    }
  }
  if (count($field_type_options) > 0) {
    $form['new'] = array(
      '#type' => 'fieldset',
      '#title' => t('Create new field'),
    );
    $form['new']['widget']['label'] = array(
      '#title' => t('Name'),
      '#type' => 'textfield',
      '#default_value' => '',
      '#description' => t('The machine-readable name of the field.<br/>Allowed characters : unaccentuated a-z, numbers and _. All other characters will be discarded.<br/>You\'ll be able to choose a human-readable label for the field on next page'),
      '#required' => TRUE,
    );
    $form['new']['field_widget_type'] = array(
      '#type' => 'radios',
      '#title' => t('Field type'),
      '#required' => TRUE,
      '#options' => $field_type_options,
      '#theme' => 'content_admin_field_add_new_field_widget_type',
    );

    $form['new']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Create field'),
    );
    $form['new']['type_name'] = array(
      '#type' => 'value',
      '#value' => $type_name,
    );
    $form['new']['field_name'] = array(
      '#type' => 'value',
      '#value' => $new_field_name,
    );
  }
  else {
    drupal_set_message(t('No field modules are enabled. You need to <a href="!modules_url">enable one</a>, such as text.module, before you can add new fields.', array('!modules_url' => url('admin/build/modules'))), 'error');
  }

  return $form;
}

function theme_content_admin_field_add_new_field_widget_type($form) {
  $field_types = _content_field_types();
  $widget_types = _content_widget_types();
  $output = '';

  $output .= '<dl>';
  foreach ($field_types as $field_name => $field_type) {
    $output .= '<dt>'. $field_type['label'] .'</dt>';
    foreach ($widget_types as $widget_name => $widget_type) {
      if (in_array($field_name, $widget_type['field types'])) {
        $output .= '<dd>'. drupal_render($form[$field_name .'-'. $widget_name]) .'</dd>';
      }
    }
  }
  $output .= '</dl>';

  return $output;
}

/**
 * Add an existing field to a content type.
 */
function _content_admin_field_add_existing_submit($form_id, $form_values) {
  $type  = content_types($form_values['type_name']);
  $field = content_fields($form_values['field_name']);
  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $columns = module_invoke($field_type['module'], 'field_settings', 'database columns', $field);

  if (is_array($columns) && count($columns)) {
    if ($field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
      $new_field = $field;
      $new_field['db_storage'] = CONTENT_DB_STORAGE_PER_FIELD;
      db_query("UPDATE {node_field} SET db_storage = %d WHERE field_name = '%s'", CONTENT_DB_STORAGE_PER_FIELD, $form_values['field_name']);
      content_alter_db_field($field, $columns, $new_field, $columns);
    }
  }

  $prior_instance = db_fetch_array(db_query("SELECT * FROM {node_field_instance} WHERE field_name = '%s'", $form_values['field_name']));
  if (!$prior_instance) {
    $prior_instance = array();
    $prior_instance['weight'] = 0;
    $prior_instance['label'] = $form_values['field_name'];
    $prior_instance['widget_type'] = '';
    $prior_instance['widget_settings'] = '';
    $prior_instance['display_settings'] = '';
    $prior_instance['description'] = '';
  }
  db_query("INSERT INTO {node_field_instance} (field_name, type_name, weight, label, widget_type, widget_settings, display_settings, description) VALUES ('%s', '%s', %d, '%s', '%s', '%s', '%s', '%s')", $form_values['field_name'], $form_values['type_name'], $prior_instance['weight'], $prior_instance['label'], $prior_instance['widget_type'], $prior_instance['widget_settings'], $prior_instance['display_settings'], $prior_instance['description']);

  drupal_set_message(t('Added field %label.', array('%label' => $prior_instance['label'])));
  content_clear_type_cache();
  return 'admin/content/types/'. $type['url_str'] .'/fields';
}

/**
 *  Field name validation for programmatic field addition that supply the field name.
 */
function _content_admin_field_add_new_validate($form_id, $form_values) {
  if ($form_values['field_name']) {
    $fields = content_fields();
    if (!empty($fields[$form_values['field_name']])) {
      form_set_error('field_name', t('The field name %field_name already exists.', array(
        '%field_name' => $form_values['field_name'])));
    }
    if (!preg_match('!^[a-z0-9_]+$!', $form_values['field_name'])) {
      form_set_error('field_name', t('The field name %field_name is invalid.', array(
        '%field_name' => $form_values['field_name'])));
    }
  }
}

/**
 * Create a new field for a content type.
 */
function _content_admin_field_add_new_submit($form_id, $form_values) {
  // Find a valid, computer-friendly field name.

  $fields = content_fields();
  $type   = content_types($form_values['type_name']);

  // Accept field name from programmed submissions if valid and it doesn't already exist.
  if ($form_values['field_name']) {
    $field_name = $form_values['field_name'];
  }
  else {
    $field_name = trim($form_values['label']);
    $field_name = drupal_strtolower($field_name);
    $field_name = str_replace(array(' ', '-'), '_', $field_name);
    $field_name = preg_replace('/[^a-z0-9_]/', '', $field_name);
    $field_name = 'field_'. $field_name;
    $field_name = substr($field_name, 0, 31);
    if (isset($fields[$field_name])) {
      $counter = 0;
      do {
        $new_name = substr($field_name, 0, 29) .'_'. $counter++;
      }   while (isset($fields[$new_name]));
      $field_name = $new_name;
    }
  }

  $field_widget_type = explode('-', $form_values['field_widget_type']);
  db_query("INSERT INTO {node_field} (field_name, type, global_settings, required, multiple, db_storage) VALUES ('%s', '%s', '%s', %d, %d, %d)", $field_name, $field_widget_type[0], serialize(array()), 0, 0, CONTENT_DB_STORAGE_PER_CONTENT_TYPE);
  db_query("INSERT INTO {node_field_instance} (field_name, type_name, weight, label, widget_type, widget_settings, display_settings, description) VALUES ('%s', '%s', %d, '%s', '%s', '%s', '%s', '%s')", $field_name, $form_values['type_name'], 0, $form_values['label'], $field_widget_type[1], serialize(array()), serialize(array()), '');

  content_clear_type_cache();

  // Create new database columns as necessary.
  $field_types = _content_field_types();
  $field_type = $field_types[$field_widget_type[0]];
  $field = content_fields($field_name);

  $columns = module_invoke($field_type['module'], 'field_settings', 'database columns', $field);
  if (is_array($columns) && count($columns)) {
    content_alter_db_field(array(), array(), $field, $columns);
  }

  drupal_set_message(t('Created field %label.', array('%label' => $form_values['label'])));
  return 'admin/content/types/'. $type['url_str'] .'/fields/'. $field_name;
}

/**
 * Menu callback; present a form for removing a field from a content type.
 */
function _content_admin_field_remove($type_name, $field_name) {
  $type = content_types($type_name);
  $field = $type['fields'][$field_name];

  $form = array();
  $form['type_name'] = array(
    '#type' => 'value',
    '#value' => $type_name,
  );
  $form['field_name'] = array(
    '#type' => 'value',
    '#value' => $field_name,
  );

  $output = confirm_form($form,
    t('Are you sure you want to remove the field %field?', array('%field' => $field['widget']['label'])),
    'admin/content/types/'. $type['url_str'] .'/fields',
    t('If you have any content left in this field, it will be lost. This action cannot be undone.'),
    t('Remove'), t('Cancel'),
    'confirm'
  );

  return $output;
}

/**
 * Remove a field from a content type.
 */
function _content_admin_field_remove_submit($form_id, $form_values) {
  $type = content_types($form_values['type_name']);
  $field = $type['fields'][$form_values['field_name']];

  if ($type && $field && $form_values['confirm']) {
    include_once('./'. drupal_get_path('module', 'content') .'/content_crud.inc');
    content_field_instance_delete($form_values);

    drupal_set_message(t('Removed field %field from %type.', array('%field' => $field['widget']['label'], '%type' => $type['name'])));

    return 'admin/content/types/'. $type['url_str'] .'/fields';
  }
}


/**
 * Menu callback; presents the field editing page.
 */
function _content_admin_field($type_name, $field_name) {
  $output = '';

  $type = content_types($type_name);
  $field = $type['fields'][$field_name];
  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$field['widget']['type']];

  $form = array();
  $form['widget'] = array(
    '#type' => 'fieldset',
    '#title' => t('Widget settings'),
    '#description' => t('These settings apply only to the %field field as it appears in the %type content type.', array('%field' => $field['widget']['label'], '%type' => $type['name'])),
  );
  $options = array();
  foreach ($widget_types as $possible_widget_name => $possible_widget_type) {
    if (in_array($field['type'], $possible_widget_type['field types'])) {
      $options[$possible_widget_name] = $possible_widget_type['label'];
    }
  }
  if (count($options) == 1) {
    $key = array_keys($options);
    $default_widget = array_pop($key);
  }
  $form['widget']['widget_type'] = array(
    '#type' => 'radios',
    '#title' => t('Widget'),
    '#options' => $options,
    '#default_value' => $field['widget']['type'] ? $field['widget']['type'] : $default_widget,
    '#required' => TRUE,
  );
  $form['widget']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $field['widget']['label'],
    '#required' => TRUE,
  );
  $form['widget']['weight'] = array(
    '#type' => 'hidden',
    '#default_value' => $field['widget']['weight'],
  );
  $additions = module_invoke($widget_type['module'], 'widget_settings', 'form', $field['widget']);
  if (is_array($additions)) {
    $form['widget'] = array_merge($form['widget'], $additions);
  }
  $form['widget']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Help text'),
    '#default_value' => $field['widget']['description'],
    '#rows' => 5,
    '#description' => t('Instructions to present to the user below this field on the editing form.'),
    '#required' => FALSE,
  );

  // Add handling for default value if not provided by field.
  if (content_handle('widget', 'default value', $field) == CONTENT_CALLBACK_DEFAULT) {
    $form['#attributes'] = array("enctype" => "multipart/form-data");
    $form['widget']['default_value_fieldset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Default value'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $default_value = is_array($field['widget']['default_value']) ? $field['widget']['default_value'] : array();
    $module = $widget_types[$field['widget']['type']]['module'];
    $function = $module .'_widget';
    if (function_exists($function)) {
      $node = array(); // TODO are there things we need to add in here ?
      // Make sure the default value is not a required field.
      $widget_field = $field;
      $widget_field['required'] = FALSE;
      $function('prepare form values', $node, $widget_field, $default_value);
      $form_element = $function('form', $node, $widget_field, $default_value);
    }
    else {
      // TODO : generate a series of textfields ?
      // why would a widget not have a hook_widget implementation ?
    }
    $form['widget']['default_value_fieldset']['default_value_widget'] = $form_element;
    $form['widget']['default_value_fieldset']['default_value_widget']['#tree'] = TRUE;
    $form['widget']['default_value_fieldset']['advanced_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Php code'),
      '#collapsible' => TRUE,
      '#collapsed' => empty($field['widget']['default_value_php']),
    );
    $db_info = content_database_info($field);
    $columns = array_keys($db_info['columns']);
    foreach ($columns as $key => $column) {
      $columns[$key] = "'$column' => value for $column";
    }
    $sample = 'array(
  0 => array('. implode(', ', $columns) .'),
  // You\'ll usually want to stop here. Provide more values
  // if you want your \'default value\' to be multi-valued :
  1 => array('. implode(', ', $columns) .'),
  2 => ...
);';
    $form['widget']['default_value_fieldset']['advanced_options']['default_value_php'] = array(
      '#type' => 'textarea',
      '#title' => t('Code'),
      '#default_value' => $field['widget']['default_value_php'],
      '#rows' => 6,
      '#tree' => TRUE,
      '#description' => t('Advanced Usage Only: PHP code that returns a default value. Should not include &lt;?php ?&gt; delimiters. If this field is filled out, the value returned by this code will override any value specified above. Expected format :<pre>!sample</pre>Using !link_devel\'s \'devel load\' tab on a %type content page might help you figure out the expected format.', array('!sample' => $sample, '!link_devel' => l('devel.module', 'http://www.drupal.org/project/devel'), '%type' => $type_name)),
    );
  }

  $form['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('Data settings'),
    '#description' => t('These settings apply to the %field field in every content type in which it appears.', array('%field' => $field['widget']['label'])),
  );
  $form['field']['required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Required'),
    '#default_value' => $field['required'],
  );
  $form['field']['multiple'] = array(
    '#type' => 'checkbox',
    '#title' => t('Multiple values'),
    '#default_value' => $field['multiple'],
  );

  $additions = module_invoke($field_type['module'], 'field_settings', 'form', $field);
  if (is_array($additions)) {
    $form['field'] = array_merge($form['field'], $additions);
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field settings'),
  );
  $form['type_name'] = array(
    '#type' => 'value',
    '#value' => $type_name,
  );
  $form['field_name'] = array(
    '#type' => 'value',
    '#value' => $field_name,
  );
  $form['field_type'] = array(
    '#type' => 'value',
    '#value' => $field['type'],
  );
  $form['module'] = array(
    '#type' => 'value',
    '#value' => implode(', ', array_unique(array($field_type['module'], $widget_type['module']))),
    );
  return $form;
}

/**
 * Validate a field's settings.
 */
function _content_admin_field_validate($form_id, $form_values, $form) {
  $type = content_types($form_values['type_name']);
  $field = $type['fields'][$form_values['field_name']];
  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$field['widget']['type']];

  module_invoke($widget_type['module'], 'widget_settings', 'validate', array_merge($field, $form_values));
  module_invoke($field_type['module'], 'field_settings', 'validate', array_merge($field, $form_values));

  // If content.module is handling the default value,
  // validate the result using the field validation.
  if (content_handle('widget', 'default value', $field) == CONTENT_CALLBACK_DEFAULT) {
    if (isset($form_values['default_value_php']) && ($php = trim($form_values['default_value_php']))) {
      ob_start();
      $return = eval($php);
      ob_end_clean();
      if (!is_array($return)) {
        $error = TRUE;
      }
      else {
        foreach ($return as $item) {
          if (!is_array($item)) {
            $error = TRUE;
            break;
          }
        }
      }
      if ($error) {
        $db_info = content_database_info($field);
        $columns = array_keys($db_info['columns']);
        foreach ($columns as $key => $column) {
          $columns[$key] = "'$column' => value for $column";
        }
        $sample = 'array(
  0 => array('. implode(', ', $columns) .'),
  // You\'ll usually want to stop here. Provide more values
  // if you want your \'default value\' to be multi-valued :
  1 => array('. implode(', ', $columns) .'),
  2 => ...
);';
        form_set_error('default_value_php', t('The default value php code returned an incorrect value<br/>Expected format : <pre>!sample</pre>Returned value : @value', array('!sample' => $sample, '@value' => print_r($return, true))));
        return;
      }
      else {
        $default_value = $return;
        $is_code = TRUE;
        form_set_value(array('#parents' => array('default_value_php')), $php);
        form_set_value(array('#parents' => array('default_value')), array());
      }
    }
    else {
      $default_value = $form_values['default_value_widget'][$field['field_name']];
      $is_code = FALSE;
      form_set_value(array('#parents' => array('default_value_php')), '');
      form_set_value(array('#parents' => array('default_value')), $default_value);
    }
    if (isset($default_value)) {
      $node = array();
      $node[$form_values['field_name']] = $default_value;
      $field['required'] = FALSE;
      $field_function = $field_type['module'] .'_field';
      $widget_function = $widget_type['module'] .'_widget';

      // If default_value is created from php code, don't run it through widget processing.
      // This way the default value code can directly create the correct array without being
      // mangled by widget processing which sometimes requires an input array in a completely different format.
      if (function_exists($widget_function) && !$is_code) {
        $widget_function('validate', $node, $field, $default_value);
        $widget_function('process form values', $node, $field, $default_value);
        // The widget processing may have altered the value, save it to be sure.
        form_set_value(array('#parents' => array('default_value')), $default_value);
      }
      if (function_exists($field_function)) {
        $field_function('validate', $node, $field, $default_value, NULL, NULL);
      }
      // The field validation routine won't set an error on the right field, so set it here.
      if (form_get_errors()) {
        if (trim($form_values['default_value_php'])) {
          form_set_error('default_value_php', t('The default value php code created @value which is invalid.', array('@value' => print_r($default_value, true))));
        }
        else {
          form_set_error('default_value', t('The default value is invalid.'));
        }
      }
    }
  }
}

/**
 * Save a field's settings after editing.
 */
function _content_admin_field_submit($form_id, $form_values) {
  $type = content_types($form_values['type_name']);
  $field = $type['fields'][$form_values['field_name']];
  $field_types = _content_field_types();
  $field_type = $field_types[$field['type']];
  $widget_types = _content_widget_types();
  $widget_type = $widget_types[$form_values['widget_type']];

  // If content.module is handling the default value,
  // initialize $widget_settings with default values,
  if (content_handle('widget', 'default value', $field) == CONTENT_CALLBACK_DEFAULT) {
    $widget_settings = array(
      'default_value' => $form_values['default_value'],
      'default_value_php' => $form_values['default_value_php'],
      );
  }

  $setting_names = module_invoke($widget_type['module'], 'widget_settings', 'save', $field);
  if (is_array($setting_names)) {
    foreach ($setting_names as $setting) {
      $widget_settings[$setting] = $form_values[$setting];
    }
  }
  $field_settings = array();
  $setting_names = module_invoke($field_type['module'], 'field_settings', 'save', $field);
  if (is_array($setting_names)) {
    foreach ($setting_names as $setting) {
      $field_settings[$setting] = $form_values[$setting];
    }
  }

  $prev_field = $field;
  $prev_columns = module_invoke($field_type['module'], 'field_settings', 'database columns', $field);

  db_query("UPDATE {node_field_instance} SET weight = %d, label = '%s', widget_type = '%s', widget_settings = '%s', description = '%s' WHERE type_name = '%s' AND field_name = '%s'", $form_values['weight'], $form_values['label'], $form_values['widget_type'], serialize($widget_settings), $form_values['description'], $form_values['type_name'], $form_values['field_name']);

  if ($form_values['multiple']) {
    $field['db_storage'] = CONTENT_DB_STORAGE_PER_FIELD;
  }
  else {
    $instances = db_result(db_query("SELECT COUNT(*) FROM {node_field_instance} WHERE field_name = '%s'", $form_values['field_name']));
    if ($instances == 1) {
      $field['db_storage'] = CONTENT_DB_STORAGE_PER_CONTENT_TYPE;
    }
  }
  db_query("UPDATE {node_field} SET required = %d, multiple = %d, global_settings = '%s', db_storage = %d WHERE field_name = '%s'", $form_values['required'], $form_values['multiple'], serialize($field_settings), $field['db_storage'], $form_values['field_name']);

  drupal_set_message(t('Saved field %field.', array('%field' => $form_values['label'])));
  content_clear_type_cache();

  $new_field = content_fields($form_values['field_name']);
  $new_columns = module_invoke($field_type['module'], 'field_settings', 'database columns', $new_field);

  if (!isset($prev_columns)) {
    $prev_columns = array();
  }
  if (!isset($new_columns)) {
    $new_columns = array();
  }
  content_alter_db_field($prev_field, $prev_columns, $new_field, $new_columns);

  return 'admin/content/types/'. $type['url_str'] .'/fields';
}

/**
 * Perform adds, alters, and drops as needed to synchronize the database with
 * new field definitions.
 */
function content_alter_db_field($previous_field, $previous_columns, $new_field, $new_columns) {
  // When adding and removing columns, we need to know what content type has an instance of the field.
  if (count($previous_columns)) {
    if (!isset($previous_field['type_name'])) {
      $previous_field['type_name'] = db_result(db_query("SELECT type_name FROM {node_field_instance} WHERE field_name = '%s'", $previous_field['field_name']));
    }
    $previous_db_info = content_database_info($previous_field);
  }
  if (count($new_columns)) {
    $new_field['type_name'] = db_result(db_query("SELECT type_name FROM {node_field_instance} WHERE field_name = '%s'", $new_field['field_name']));
    $new_db_info = content_database_info($new_field);
  }

  if (!count($new_columns)) {
    if (count($previous_columns)) {
      if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
        db_query('DROP TABLE {'. $previous_db_info['table'] .'}');
      }
      else {
        foreach ($previous_db_info['columns'] as $column => $attributes) {
          db_query('ALTER TABLE {'. $previous_db_info['table'] .'} DROP '. $attributes['column']);
        }
      }
    }
    return;
  }

  if ($new_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
    if (!count($previous_columns) || $previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
      // New columns with per-field storage; need to add a table.
      if ($new_field['multiple']) {
        switch ($GLOBALS['db_type']) {
          case 'mysql':
          case 'mysqli':
            db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                vid int unsigned NOT NULL default '0',
                delta int unsigned NOT NULL default '0',
                nid int unsigned NOT NULL default '0',
                PRIMARY KEY (vid,delta)
              ) /*!40100 DEFAULT CHARACTER SET utf8 */");
            break;

          case 'pgsql':
            db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                vid int_unsigned NOT NULL default '0',
                delta int_unsigned NOT NULL default '0',
                nid int_unsigned NOT NULL default '0',
                PRIMARY KEY (vid,delta)
              )");
            break;
        }
      }
      else {
        switch ($GLOBALS['db_type']) {
          case 'mysql':
          case 'mysqli':
            db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                vid int unsigned NOT NULL default '0',
                nid int unsigned NOT NULL default '0',
                PRIMARY KEY (vid)
              ) /*!40100 DEFAULT CHARACTER SET utf8 */");
            break;

          case 'pgsql':
            db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                vid int_unsigned NOT NULL default '0',
                nid int_unsigned NOT NULL default '0',
                PRIMARY KEY (vid)
              )");
            break;
        }
      }
    }

    if (count($previous_columns) && $previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
      // Already using per-field storage; change multiplicity if needed.
      if ($previous_field['multiple'] && !$new_field['multiple']) {
        db_query('DELETE FROM {'. $new_db_info['table'] .'} WHERE delta != 0');
        db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP delta');
        switch ($GLOBALS['db_type']) {
          case 'mysql':
          case 'mysqli':
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP PRIMARY KEY');
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} ADD PRIMARY KEY (vid)');
            break;
          case 'pgsql':
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP CONSTRAINT {'. $new_db_info['table'] .'}_pkey');
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} ADD PRIMARY KEY (vid)');
            break;
        }
      }
      else if (!$previous_field['multiple'] && $new_field['multiple']) {
        content_db_add_column($new_db_info['table'], 'delta', 'int', array('unsigned' => TRUE, 'not null' => TRUE, 'default' => 0));
        switch ($GLOBALS['db_type']) {
          case 'mysql':
          case 'mysqli':
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP PRIMARY KEY');
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} ADD PRIMARY KEY (vid,delta)');
            break;
          case 'pgsql':
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP CONSTRAINT {'. $new_db_info['table'] .'}_pkey');
            db_query('ALTER TABLE {'. $new_db_info['table'] .'} ADD PRIMARY KEY (vid,delta)');
            break;
        }
      }
    }
  }

  // Add new columns and change modified columns.
  foreach ($new_columns as $column => $attributes) {
    $column_name = $new_field['field_name'] .'_'. $column;
    if (!isset($previous_columns[$column]) || $previous_field['db_storage'] != $new_field['db_storage']) {
      if (!db_table_exists($new_db_info['table'])) {
        if ($new_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
          if ($new_field['multiple']) {
            switch ($GLOBALS['db_type']) {
              case 'mysql':
              case 'mysqli':
                db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                  vid int unsigned NOT NULL default '0',
                  delta int unsigned NOT NULL default '0',
                  nid int unsigned NOT NULL default '0',
                  PRIMARY KEY (vid,delta)
                ) /*!40100 DEFAULT CHARACTER SET utf8 */");
                break;

              case 'pgsql':
                db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                  vid int_unsigned NOT NULL default '0',
                  delta int_unsigned NOT NULL default '0',
                  nid int_unsigned NOT NULL default '0',
                  PRIMARY KEY (vid,delta)
                )");
                break;
            }

          }
          else {
            switch ($GLOBALS['db_type']) {
              case 'mysql':
              case 'mysqli':
                db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                  vid int unsigned NOT NULL default '0',
                  nid int unsigned NOT NULL default '0',
                  PRIMARY KEY (vid)
                ) /*!40100 DEFAULT CHARACTER SET utf8 */");
                break;

                case 'pgsql':
                  db_query("CREATE TABLE {". $new_db_info['table'] ."} (
                    vid int_unsigned NOT NULL default '0',
                    nid int_unsigned NOT NULL default '0',
                    PRIMARY KEY (vid)
                  )");
                  break;
            }
          }  // end: if ($new_field['multiple'])
        }  // end: if ($new_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE)
      }
      content_db_add_column($new_db_info['table'], $column_name, $attributes['type'], $attributes);
    }
    else {
      if ($attributes != $previous_columns[$column]) {
        content_db_change_column($new_db_info['table'], $column_name, $column_name, $attributes['type'], $attributes);
      }
    }
  }

  if (count($previous_columns) && count($new_columns)) {
    // Remove obsolete columns.
    foreach ($previous_columns as $column => $attributes) {
      $column_name = $previous_field['field_name'] .'_'. $column;
      if (!isset($new_columns[$column])) {
        db_query('ALTER TABLE {'. $new_db_info['table'] .'} DROP '. $column_name);
      }
    }

    // Migrate data from one storage type to another
    // We check if the previous table still exists (avoid problems during upgrades from older db schemes)
    if (db_table_exists($previous_db_info['table'])) {
      // Migrate data from per-content-type storage.
      if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE && $new_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD) {
        $columns = array();
        foreach ($previous_db_info['columns'] as $column => $attributes) {
          $columns[] = $attributes['column'];
        }
        if ($new_field['multiple']) {
          db_query('INSERT INTO {'. $new_db_info['table'] .'} (vid, nid, delta, '. implode(', ', $columns) .') SELECT vid, nid, 0, '. implode(', ', $columns) .' FROM {'. $previous_db_info['table'] .'}');
        }
        else {
          db_query('INSERT INTO {'. $new_db_info['table'] .'} (vid, nid, '. implode(', ', $columns) .') SELECT vid, nid, '. implode(', ', $columns) .' FROM {'. $previous_db_info['table'] .'}');
        }
        foreach ($columns as $column_name) {
          db_query('ALTER TABLE {'. $previous_db_info['table'] .'} DROP '. $column_name);
        }
      }

      // Migrate data from per-field storage.
      if ($previous_field['db_storage'] == CONTENT_DB_STORAGE_PER_FIELD && $new_field['db_storage'] == CONTENT_DB_STORAGE_PER_CONTENT_TYPE) {
        $column_names = array();
        $column_placeholders_default = array();
        $column_assignments_default = array();
        foreach ($new_db_info['columns'] as $column => $attributes) {
          $column_names[] = $attributes['column'];
          if (in_array($attributes['type'], array('int', 'mediumint', 'tinyint', 'bigint', 'float'))) {
            $column_placeholders_default[] = '%d';
            $column_assignments_default[] = $attributes['column'] .' = %d';
          }
          else {
            $column_placeholders_default[] = "'%s'";
            $column_assignments_default[] = $attributes['column'] ." = '%s'";
          }
        }
        if ($previous_field['multiple']) {
          $result = db_query('SELECT '. implode(', ', $column_names) .', vid, nid FROM {'. $previous_db_info['table'] .'} WHERE delta = 0');
        }
        else {
          $result = db_query('SELECT '. implode(', ', $column_names) .', vid, nid FROM {'. $previous_db_info['table'] .'}');
        }
        while ($data = db_fetch_array($result)) {
          $column_assignments = $column_assignments_default;
          $column_placeholders = $column_placeholders_default;
          // search for NULL values and replace assignments and placeholders accordingly
          foreach ($data as $key => $value) {
            if (is_null($value)) {
              $pos = array_search($key, $column_names);
              $column_assignments[$pos] = $key ."= %s";
              $column_placeholders[$pos] = "%s";
              $data[$key] = 'NULL';
            }
          }
          if (db_result(db_query('SELECT COUNT(*) FROM {'. $new_db_info['table'] .'} WHERE vid = %d AND nid = %d', $data['vid'], $data['nid']))) {
            db_query('UPDATE {'. $new_db_info['table'] .'} SET '. implode(', ', $column_assignments) .' WHERE vid = %d AND nid = %d', $data);
          }
          else {
            db_query('INSERT INTO {'. $new_db_info['table'] .'} ('. implode(', ', $column_names) .', vid, nid) VALUES ('. implode(', ', $column_placeholders) .', %d, %d)', $data);
          }
        }
        db_query('DROP TABLE {'. $previous_db_info['table'] .'}');
      }
    }
  }
}

/**
 * Add a column to a database table.
 *
 * @param $table
 *   Name of the table, without {}
 * @param $column
 *   Name of the column
 * @param $type
 *   Type of column
 * @param $attributes
 *   Additional optional attributes. Recognized attributes:
 *     not null => TRUE|FALSE
 *     default  => NULL|FALSE|value (with or without '', it won't be added)
 */
function content_db_add_column($table, $column, $type, $attributes = array()) {
  switch ($GLOBALS['db_type']) {
    case 'pgsql':
      $mappings = array('int' => 'integer', 'mediumint' => 'integer', 'bigint' => 'integer',
        'tinyint' => 'smallint',
        'float' => 'float',
        'varchar' => 'varchar',
        'text' => 'text', 'mediumtext' => 'text', 'longtext' => 'text');
      if (isset($mappings[$type])) {
        $type = $mappings[$type];
      }
      else {
        watchdog('database', t('No PostgreSQL mapping found for %type data type.', array('%type' => $type)), WATCHDOG_WARNING);
      }
      if ($type != 'varchar') {
        unset($attributes['length']);
      }
      break;
  }

  if (array_key_exists('not null', $attributes) && $attributes['not null']) {
    $not_null = 'NOT NULL';
  }
  if (array_key_exists('default', $attributes)) {
    if (is_null($attributes['default'])) {
      $default_val = 'NULL';
      $default = 'default NULL';
    }
    elseif ($attributes['default'] === FALSE) {
      $default = '';
    }
    else {
      $default_val = "$attributes[default]";
      $default = "default $attributes[default]";
    }
  }
  if (array_key_exists('length', $attributes)) {
    $type .= '('. $attributes['length'] .')';
  }
  if (array_key_exists('unsigned', $attributes) && $attributes['unsigned']) {
    switch ($GLOBALS['db_type']) {
      case 'pgsql':
        $type = str_replace('integer', 'int_unsigned', $type);
        break;
      default:
        $type .= ' unsigned';
        break;
    }
  }
  switch ($GLOBALS['db_type']) {
    case 'pgsql':
      db_query("ALTER TABLE {". $table ."} ADD $column $type");
      if ($default) {
        db_query("ALTER TABLE {". $table ."} ALTER $column SET $default");
      }
      if ($not_null) {
        if ($default) {
          db_query("UPDATE {". $table ."} SET $column = $default_val");
        }
        db_query("ALTER TABLE {". $table ."} ALTER $column SET NOT NULL");
      }
      break;

    case 'mysql':
    case 'mysqli':
      // MySQL allows no DEFAULT value for text (and blob) columns
      if (in_array($type, array('text', 'mediumtext', 'longtext'))) {
        $default = '';
        // We also allow NULL values to account for CCK's per field INSERTs
        $not_null = '';
      }
      db_query('ALTER TABLE {'. $table .'} ADD COLUMN '. $column .' '. $type .' '. $not_null .' '. $default);
      break;
  }
}

/**
 * Change a column definition.
 *
 * Remember that changing a column definition involves adding a new column
 * and dropping an old one. This means that any indices, primary keys and
 * sequences from serial-type columns are dropped and might need to be
 * recreated.
 *
 * @param $table
 *   Name of the table, without {}
 * @param $column
 *   Name of the column to change
 * @param $column_new
 *   New name for the column (set to the same as $column if you don't want to change the name)
 * @param $type
 *   Type of column
 * @param $attributes
 *   Additional optional attributes. Recognized attributes:
 *     not null => TRUE|FALSE
 *     default  => NULL|FALSE|value (with or without '', it won't be added)
 */
function content_db_change_column($table, $column, $column_new, $type, $attributes = array()) {
  switch ($GLOBALS['db_type']) {
    case 'pgsql':
      $mappings = array('int' => 'integer', 'mediumint' => 'integer', 'bigint' => 'integer',
        'tinyint' => 'smallint',
        'float' => 'float',
        'varchar' => 'varchar',
        'text' => 'text', 'mediumtext' => 'text', 'longtext' => 'text');
      if (isset($mappings[$type])) {
        $type = $mappings[$type];
      }
      else {
        watchdog('database', t('No PostgreSQL mapping found for %type data type.', array('%type' => $type)), WATCHDOG_WARNING);
      }
      if ($type != 'varchar') {
        unset($attributes['length']);
      }
      break;

    case 'mysql':
    case 'mysqli':
      break;
  }

  if (array_key_exists('not null', $attributes) and $attributes['not null']) {
    $not_null = 'NOT NULL';
  }
  if (array_key_exists('default', $attributes)) {
    if (is_null($attributes['default'])) {
      $default_val = 'NULL';
      $default = 'default NULL';
    }
    elseif ($attributes['default'] === FALSE) {
      $default = '';
    }
    else {
      $default_val = "$attributes[default]";
      $default = "default $attributes[default]";
    }
  }
  if (array_key_exists('length', $attributes)) {
    $type .= '('. $attributes['length'] .')';
  }
  if (array_key_exists('unsigned', $attributes) && $attributes['unsigned']) {
    switch ($GLOBALS['db_type']) {
      case 'pgsql':
        $type = str_replace('integer', 'int_unsigned', $type);
        break;
      default:
        $type .= ' unsigned';
        break;
    }
  }

  switch ($GLOBALS['db_type']) {
    case 'pgsql':
      db_query("ALTER TABLE {". $table ."} RENAME $column TO ". $column ."_old");
      db_query("ALTER TABLE {". $table ."} ADD $column_new $type");
      db_query("UPDATE {". $table ."} SET $column_new = ". $column ."_old");
      if ($default) {
        db_query("ALTER TABLE {". $table ."} ALTER $column_new SET $default");
      }
      if ($not_null) {
        db_query("ALTER TABLE {". $table ."} ALTER $column_new SET NOT NULL");
      }
      db_query("ALTER TABLE {". $table ."} DROP ". $column ."_old");
      break;

    case 'mysql':
    case 'mysqli':
      // MySQL allows no DEFAULT value for text (and blob) columns
      if (in_array($type, array('text', 'mediumtext', 'longtext'))) {
        $default = '';
        // We also allow NULL values to account for CCK's per field INSERTs
        $not_null = '';
      }
      db_query('ALTER TABLE {'. $table .'} CHANGE '. $column .' '. $column_new .' '. $type .' '. $not_null .' '. $default);
      break;
  }
}