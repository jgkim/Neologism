<?php
// $Id: field.php,v 1.7.2.8 2007/03/01 00:20:35 yched Exp $

/**
 * @file
 * These hooks are defined by field modules, modules that define a new kind
 * of field for insertion in a content type.
 *
 * Field hooks are typically called by content.module using _content_field_invoke().
 *
 * Widget module hooks are also defined here; the two go hand-in-hand, often in
 * the same module (though they are independent).
 *
 * Widget hooks are typically called by content.module using _content_widget_invoke().
 */

/**
 * @addtogroup hooks
 * @{
 */


/**
 * Declare information about a field type.
 *
 * @return
 *   An array keyed by field type name. Each element of the array is an associative
 *   array with these keys and values:
 *   - "label": The human-readable label for the field type.
 */
function hook_field_info() {
  return array(
    'number_integer' => array('label' => 'Integer'),
    'number_decimal' => array('label' => 'Decimal'),
  );
}

/**
 * Handle the parameters for a field.
 *
 * @param $op
 *   The operation to be performed. Possible values:
 *   - "form": Display the field settings form.
 *   - "validate": Check the field settings form for errors.
 *   - "save": Declare which fields to save back to the database.
 *   - "database columns": Declare the columns that content.module should create
 *     and manage on behalf of the field. If the field module wishes to handle
 *     its own database storage, this should be omitted.
 *   - "callbacks": Describe the field's behaviour regarding hook_field operations.
 *   - "tables" : Declare the Views tables informations for the field.
 *     Use this operator only if you need to override CCK's default general-purpose
 *     implementation.
 *     In this case, it is probably a good idea to use the default definitions
 *     returned by content_views_field_tables($field) as a start point for your own
 *     definitions.
 *   - "arguments" : Declare the Views arguments informations for the field.
 *     Use this operator only if you need to override CCK's default general-purpose
 *     implementation.
 *     In this case, it is probably a good idea to use the default definitions
 *     returned by content_views_field_arguments($field) as a start point for your own
 *     definitions.
 *   - "filters": Declare the Views filters available for the field.
 *     (this is used in CCK's default Views tables definition)
 *     They always apply to the first column listed in the "database columns"
 *     array.
 * @param $field
 *   The field on which the operation is to be performed.
 * @return
 *   This varies depending on the operation.
 *   - "form": an array of form elements to add to
 *     the settings page.
 *   - "validate": no return value. Use form_set_error().
 *   - "save": an array of names of form elements to
 *     be saved in the database.
 *   - "database columns": an array keyed by column name, with arrays of column
 *     information as values. This column information must include "type", the
 *     MySQL data type of the column, and may also include a "sortable" parameter
 *     to indicate to views.module that the column contains ordered information.
 *     Details of other information that can be passed to the database layer can
 *     be found at content_db_add_column().
 *   - "callbacks": an array describing the field's behaviour regarding hook_field
 *     operations. The array is keyed by hook_field operations ('view', 'validate'...)
 *     and has the following possible values :
 *       CONTENT_CALLBACK_NONE     : do nothing for this operation
 *       CONTENT_CALLBACK_CUSTOM   : use the behaviour in hook_field(operation)
 *       CONTENT_CALLBACK_DEFAULT  : use content.module's default bahaviour
 *     Note : currently only the 'view' operation implements this feature.
 *     All other field operation implemented by the module _will_ be executed
 *     no matter what.
 *   - "tables": an array of 'tables' definitions as expected by views.module
 *     (see Views Documentation).
 *   - "arguments": an array of 'arguments' definitions as expected by views.module
 *     (see Views Documentation).
 *   - "filters": an array of 'filters' definitions as expected by views.module
 *     (see Views Documentation).
 *     When providing several filters, it is recommended to use the 'name'
 *     attribute in order to let the user distinguish between them. If no 'name'
 *     is specified for a filter, the key of the filter will be used instead.
 */
function hook_field_settings($op, $field) {
  switch ($op) {
    case 'form':
      $form = array();
      $form['max_length'] = array(
        '#type' => 'textfield',
        '#title' => t('Maximum length'),
        '#default_value' => $field['max_length'] ? $field['max_length'] : '',
        '#required' => FALSE,
        '#description' => t('The maximum length of the field in characters. Leave blank for an unlimited size.'),
      );
      return $form;

    case 'save':
      return array('text_processing', 'max_length', 'allowed_values');

    case 'database columns':
      $columns = array(
        'value' => array('type' => 'varchar', 'not null' => TRUE, 'default' => "''", 'sortable' => TRUE),
        'format' => array('type' => 'int', 'length' => 10, 'unsigned' => TRUE, 'not null' => TRUE, 'default' => 0),
      );
      if ($field['max_length'] == 0 || $field['max_length'] > 255) {
        $columns['value']['type'] = 'longtext';
      }
      else {
        $columns['value']['length'] = $field['max_length'];
      }
      return $columns;

    case 'callbacks':
      return array(
        'view' => CONTENT_CALLBACK_CUSTOM,
      );

    case 'tables':
      $tables = content_views_field_tables($field);
      // whatever additions / modifications needed on the default definitions
      return $tables;

    case 'arguments':
      $arguments = content_views_field_arguments($field);
      // whatever additions / modifications needed on the default definitions
      return $arguments;

    case 'filters':
      return array(
        'substring' => array(
          'operator' => 'views_handler_operator_like',
          'handler' => 'views_handler_filter_like',
        ),
        'alpha_order' => array(
          'name' => 'alphabetical order',
          'operator' => 'views_handler_operator_gtlt',
        ),
      );

  }
}

/**
 * Define the behavior of a field type.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "load": The node is about to be loaded from the database. This hook
 *     should be used to load the field.
 *   - "view": The node is about to be presented to the user. The module
 *     should prepare and return an HTML string containing a default
 *     representation of the field.
 *     It will be called only if 'view' was set to TRUE in hook_field_settings('callbacks')
 *   - "validate": The user has just finished editing the node and is
 *     trying to preview or submit it. This hook can be used to check or
 *     even modify the node. Errors should be set with form_set_error().
 *   - "submit": The user has just finished editing the node and the node has
 *     passed validation. This hook can be used to modify the node.
 *   - "insert": The node is being created (inserted in the database).
 *   - "update": The node is being updated.
 *   - "delete": The node is being deleted.
 * @param &$node
 *   The node the action is being performed on. This argument is passed by
 *   reference for performance only; do not modify it.
 * @param $field
 *   The field the action is being performed on.
 * @param &$node_field
 *   The contents of the field in this node. Changes to this variable will
 *   be saved back to the node object.
 * @return
 *   This varies depending on the operation.
 *   - The "load" operation should return an object containing extra values
 *     to be merged into the node object.
 *   - The "view" operation should return a string containing an HTML
 *     representation of the field data.
 *   - The "insert", "update", "delete", "validate", and "submit" operations
 *     have no return value.
 *
 * In most cases, only "validate" operations is relevant ; the rest
 * have default implementations in content_field() that usually suffice.
 */
function hook_field($op, &$node, $field, &$node_field, $teaser, $page) {
  switch ($op) {
    case 'view':
      $context = $teaser ? 'teaser' : 'full';
      $formatter = isset($field['display_settings'][$context]['format']) ? $field['display_settings'][$context]['format'] : 'default';
      $items = array();
      foreach ($node_field as $delta => $item) {
        $items[$delta]['view'] = content_format($field, $item, $formatter, $node);
      }
      return theme('field', $node, $field, $items, $teaser, $page);

    case 'validate':
      $allowed_values = text_allowed_values($field);

      if (is_array($items)) {
        foreach ($items as $delta => $item) {
          $error_field = $field['field_name']. ']['. $delta.'][value';
          if ($item['value'] != '') {
            if (count($allowed_values) && !array_key_exists($item['value'], $allowed_values)) {
              form_set_error($error_field, t('Illegal value for %name.', array('%name' => t($field['widget']['label']))));
            }
          }
        }
      }
      break;
  }
}

/**
 * Declare information about a formatter.
 *
 * @return
 *   An array keyed by formatter name. Each element of the array is an associative
 *   array with these keys and values:
 *   - "label": The human-readable label for the formatter.
 *   - "field types": An array of field type names that can be displayed using
 *     this formatter.
 */
function hook_field_formatter_info() {
  return array(
    'default' => array(
      'label' => 'Default',
      'field types' => array('text'),
    ),
    'plain' => array(
      'label' => 'Plain text',
      'field types' => array('text'),
    ),
    'trimmed' => array(
      'label' => 'Trimmed',
      'field types' => array('text'),
    ),
  );
}

/**
 * Prepare an individual item for viewing in a browser.
 *
 * @param $field
 *   The field the action is being performed on.
 * @param $item
 *   An array, keyed by column, of the data stored for this item in this field.
 * @param $formatter
 *   The name of the formatter being used to display the field.
 * @param $node
 *   The node object, for context. Will be NULL in some cases.
 *   Warning : when displaying field retrieved by Views, $node will not
 *   be a "full-fledged" node object, but an object containg the data returned
 *   by the Views query (at least nid, vid, changed)
 * @return
 *   An HTML string containing the formatted item.
 *
 * In a multiple-value field scenario, this function will be called once per
 * value currently stored in the field. This function is also used as the handler
 * for viewing a field in a views.module tabular listing.
 *
 * It is important that this function at the minimum perform security
 * transformations such as running check_plain() or check_markup().
 */
function hook_field_formatter($field, $item, $formatter, $node) {
  if (!isset($item['value'])) {
    return '';
  }
  if ($field['text_processing']) {
    $text = check_markup($item['value'], $item['format'], is_null($node) || isset($node->in_preview));
  }
  else {
    $text = check_plain($item['value']);
  }

  switch ($formatter) {
    case 'plain':
      return strip_tags($text);

    case 'trimmed':
      return node_teaser($text, $field['text_processing'] ? $item['format'] : NULL);

    default:
      return $text;
  }
}


/**
 * Declare information about a widget.
 *
 * @return
 *   An array keyed by widget name. Each element of the array is an associative
 *   array with these keys and values:
 *   - "label": The human-readable label for the widget.
 *   - "field types": An array of field type names that can be edited using
 *     this widget.
 */
function hook_widget_info() {
  return array(
    'text' => array(
      'label' => 'Text Field',
      'field types' => array('text'),
    ),
  );
}

/**
 * Handle the parameters for a widget.
 *
 * @param $op
 *   The operation to be performed. Possible values:
 *   - "form": Display the widget settings form.
 *   - "validate": Check the widget settings form for errors.
 *   - "save": Declare which pieces of information to save back to the database.
 *   - "callbacks": Describe the widget's behaviour regarding hook_widget operations.
 * @param $widget
 *   The widget on which the operation is to be performed.
 * @return
 *   This varies depending on the operation.
 *   - "form": an array of form elements to add to the settings page.
 *   - "validate": no return value. Use form_set_error().
 *   - "save": an array of names of form elements to be saved in the database.
 *   - "callbacks": an array describing the widget's behaviour regarding hook_widget
 *     operations. The array is keyed by hook_widget operations ('form', 'validate'...)
 *     and has the following possible values :
 *       CONTENT_CALLBACK_NONE     : do nothing for this operation
 *       CONTENT_CALLBACK_CUSTOM   : use the behaviour in hook_widget(operation)
 *       CONTENT_CALLBACK_DEFAULT  : use content.module's default bahaviour
 *     Note : currently only the 'default value' operation implements this feature.
 *     All other widget operation implemented by the module _will_ be executed
 *     no matter what.
 */
function hook_widget_settings($op, $widget) {
  switch ($op) {
    case 'form':
      $form = array();
      $form['rows'] = array(
        '#type' => 'textfield',
        '#title' => t('Rows'),
        '#default_value' => $widget['rows'] ? $widget['rows'] : 1,
        '#required' => TRUE,
      );
      return $form;

    case 'validate':
      if (!is_numeric($widget['rows']) || intval($widget['rows']) != $widget['rows'] || $widget['rows'] <= 0) {
        form_set_error('rows', t('"Rows" must be a positive integer.'));
      }
      break;

    case 'save':
      return array('rows');

    case 'callbacks':
      return array(
        'default value' => CONTENT_CALLBACK_NONE,
      );
  }
}

/**
 * Define the behavior of a widget.
 *
 * @param $op
 *   What kind of action is being performed. Possible values:
 *   - "prepare form values": The editing form will be displayed. The widget
 *     should perform any conversion necessary from the field's native storage
 *     format into the storage used for the form. Convention dictates that the
 *     widget's version of the data should be stored beginning with "default".
 *   - "form": The node is being edited, and a form should be prepared for
 *     display to the user.
 *   - "validate": The user has just finished editing the node and is
 *     trying to preview or submit it. This hook can be used to check or
 *     even modify the node. Errors should be set with form_set_error().
 *   - "process form values": The inverse of the prepare operation. The widget
 *     should convert the data back to the field's native format.
 *   - "submit": The user has just finished editing the node and the node has
 *     passed validation. This hook can be used to modify the node.
 * @param &$node
 *   The node the action is being performed on. This argument is passed by
 *   reference for performance only; do not modify it.
 * @param $field
 *   The field the action is being performed on.
 * @param &$node_field
 *   The contents of the field in this node. Changes to this variable will
 *   be saved back to the node object.
 * @return
 *   This varies depending on the operation.
 *   - The "form" operation should return an array of form elements to display.
 *   - Other operations have no return value.
 */
function hook_widget($op, &$node, $field, &$node_field) {
  switch ($op) {
    case 'prepare form values':
      if ($field['multiple']) {
        $node_field_transposed = content_transpose_array_rows_cols($node_field);
        $node_field['default nids'] = $node_field_transposed['nid'];
      }
      else {
        $node_field['default nids'] = array($node_field['nid']);
      }
      break;

    case 'form':
      $form = array();

      $form[$field['field_name']] = array('#tree' => TRUE);
      $form[$field['field_name']]['nids'] = array(
        '#type' => 'select',
        '#title' => t($field['widget']['label']),
        '#default_value' => $node_field['default nids'],
        '#multiple' => $field['multiple'],
        '#options' => _nodereference_potential_references($field),
        '#required' => $field['required'],
        '#description' => $field['widget']['description'],
      );
      return $form;

    case 'process form values':
      if ($field['multiple']) {
        $node_field = content_transpose_array_rows_cols(array('nid' => $node_field['nids']));
      }
      else {
        $node_field['nid'] = is_array($node_field['nids']) ? reset($node_field['nids']) : $node_field['nids'];
      }
      break;
  }
}



/**
 * @} End of "addtogroup hooks".
 */
