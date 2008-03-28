<?php
// $Id: content_views.inc,v 1.2.2.15 2007/06/12 19:57:02 yched Exp $

/**
 * @file
 * Interface between content.module and views.module.
 */

/**
 * Implementation of hook_views_tables().
 *
 * Exposes all fields to the views system.
 */
function content_views_tables() {
  $field_types = _content_field_types();
  $return = array();
  foreach (content_fields() as $field) {
    $module = $field_types[$field['type']]['module'];
    $result = module_invoke($module, 'field_settings', 'tables', $field);
    if (empty($result)) {
      $result = content_views_field_tables($field);
    }
    if (is_array($result)) {
      $return = array_merge($return, $result);
    }
  }
  return $return;
}

function content_views_field_tables($field) {
  $field_types = _content_field_types();
  $db_info = content_database_info($field);

  if (count($db_info['columns'])) {
    $table = array();

    $table['name'] = $db_info['table'];
    $table['join'] = array(
      'left' => array(
        'table' => 'node',
        'field' => 'vid',
      ),
      'right' => array(
        'field' => 'vid',
      ),
    );

    $module = $field_types[$field['type']]['module'];

    $formatters = array();
    if (is_array($field_types[$field['type']]['formatters'])) {
      foreach ($field_types[$field['type']]['formatters'] as $name => $info) {
        $formatters[$name] = t($info['label']);
      }
    }

    $columns = $db_info['columns'];
    $main_column = array_shift($columns);
    $addlfields = array();
    foreach ($columns as $column => $attributes) {
      $addlfields[] = $attributes['column'];
    }

    $table['fields'] = array();
    $table['fields'][$main_column['column']] = array(
      'name' => $field_types[$field['type']]['label'] .': '. $field['widget']['label'] .' ('. $field['field_name'] .')',
      'addlfields' => $addlfields,
      'sortable' => isset($main_column['sortable']) ? $main_column['sortable'] : FALSE,
      'query_handler' => 'content_views_field_query_handler',
      'handler' => array('content_views_field_handler_group' => t('Group multiple values'),
                         'content_views_field_handler_ungroup' => t('Do not group multiple values')),
      'option' => array('#type' => 'select', '#options' => $formatters),
      'content_db_info' => $db_info,
      'content_field' => $field,
      'content_field_module' => $module,
    );
    if (isset($main_column['sortable']) && $main_column['sortable']) {
      $table['sorts'] = array();
      $table['sorts'][$main_column['column']] = array(
        'name' => $field_types[$field['type']]['label'] .': '. $field['widget']['label'] .' ('. $field['field_name'] .')',
        'field' => $main_column['column'],
        'content_db_info' => $db_info,
        'content_field' => $field,
        'content_field_module' => $module,
      );
    }

    $filters = module_invoke($module, 'field_settings', 'filters', $field);
    if (is_array($filters) && count($filters)) {
      $table['filters'] = array();
      foreach ($filters as $key => $filter) {
        $filter_name = '';
        if (count($filters) > 1) {
          $filter_name = (!empty($filter['name'])) ? $filter['name'] : $key;
          $filter_name = ' - '.$filter_name;
        }
        $name = $field_types[$field['type']]['label'] .': '. $field['widget']['label'] . $filter_name .' ('. $field['field_name'] .')';
        $init = array(
          'name' => $name,
          'field' => $main_column['column'],
          'content_db_info' => $db_info,
          'content_field' => $field,
          'content_field_module' => $module,
        );
        $table['filters'][$main_column['column'] .'_'. $key] = array_merge($filter, $init);
      }
    }

    // We don't use $db_info['table'] for the key, since that may change during
    // the lifetime of the field and we don't want to require users to redefine
    // their views.
    return array('node_data_'. $field['field_name'] => $table);
  }
}

function content_views_field_query_handler($field, &$fieldinfo, &$query) {
  if ($field['handler'] == 'content_views_field_handler_group') {
    // we manage the addition of fields ourselves
    // if not multiple field, add the columns to the query
    if (!$fieldinfo['content_field']['multiple']) {
      $query->add_field($field['field'], $field['tablename'], $field['queryname']);
      foreach ($fieldinfo['addlfields'] as $name)
        $query->add_field($name, $field['tablename'], "$field[tablename]_$name");
    }
    // if multiple field, no field gets added (they are retrieved later
    // in content_views_field_handler in order to avoid duplicate results)
    else {
      // if the field is sortable (table sort), then we have to join the table
      // ("Node: Distinct" will be required to avoid duplicates...)
      if ($field['sortable']) {
        $query->ensure_table($field['tablename']);
      }
    }
    // make sure views default query builder does not add anything
    $fieldinfo['notafield'] = true;
    unset($fieldinfo['addlfields']);
  }
}

function content_views_field_handler_group($field_info, $field_data, $value, $data) {
  $field = $field_info['content_field'];
  $items = array();
  if ($field['multiple']) {
    foreach ($field_info['content_db_info']['columns'] as $column => $attributes) {
      $query_columns[] = "node_data_$field[field_name].$attributes[column] AS $column";
    }
    $query = "SELECT ". implode(', ', $query_columns) .
             " FROM {node} node".
             " LEFT JOIN {". $field_info['content_db_info']['table'] ."} node_data_$field[field_name] ON node.vid = node_data_$field[field_name].vid".
             " WHERE node.nid = ". $data->nid .
             " ORDER BY node_data_$field[field_name].delta";
    $result = db_query(db_rewrite_sql($query, 'node'));
    while ($item = db_fetch_array($result)) {
      $items[] = content_format($field, $item, $field_data['options'], $data);
    }
    return theme('content_view_multiple_field', $items, $field, $data);
  }
  else {
    return content_views_field_handler_ungroup($field_info, $field_data, $value, $data);
  }
}

function content_views_field_handler_ungroup($field_info, $field_data, $value, $data) {
  $field = $field_info['content_field'];
  $item = array();
  foreach ($field_info['content_db_info']['columns'] as $column => $attributes) {
    $view_column_name = $field_data['tablename'] .'_'. $attributes['column'];
    $item[$column] = $data->$view_column_name;
  }
  return content_format($field, $item, $field_data['options'], $data);
}

function theme_content_view_multiple_field($items, $field, $data) {
  foreach ($items as $item) {
    $output .= '<div class="field-item">'. $item .'</div>';
  }
  return $output;
}

/**
 * Implementation of hook_views_arguments().
 *
 * Exposes all fields as filterable arguments.
 */
function content_views_arguments() {
  $field_types = _content_field_types();
  $return = array();
  foreach (content_fields() as $field) {
    $module = $field_types[$field['type']]['module'];
    $result = module_invoke($module, 'field_settings', 'arguments', $field);
    if (empty($result)) {
      $result = content_views_field_arguments($field);
    }
    if (is_array($result)) {
      $return = array_merge($return, $result);
    }
  }
  return $return;
}

function content_views_field_arguments($field) {
  $field_types = _content_field_types();
  $db_info = content_database_info($field);
  if (count($db_info['columns'])) {
    $argument = array();
    $argument['name'] = $field_types[$field['type']]['label'] .': '. $field['widget']['label'] .' ('. $field['field_name'] .')';
    $argument['handler'] = 'content_views_argument_handler';

    return array('content: '. $field['field_name'] => $argument);
  }
}

/**
 * Perform filtering by an argument for field data stored via content.module.
 */
function content_views_argument_handler($op, &$query, $argtype, $arg = '') {
  if ($op == 'filter') {
    $field_name = substr($argtype['type'], 9);
  }
  else {
    $field_name = substr($argtype, 9);
  }

  $field = content_fields($field_name);
  $db_info = content_database_info($field);
  $main_column = reset($db_info['columns']);

  // The table name used here is the Views alias for the table, not the actual
  // table name.
  $table = 'node_data_'. $field['field_name'];

  switch ($op) {
    case 'summary':
      $query->ensure_table($table);
      $query->add_field($main_column['column'], $table);
      return array('field' => $table .'.'. $main_column['column']);
      break;

    case 'sort':
      break;

    case 'filter':
      $query->ensure_table($table);
      switch ($main_column['type']) {
        case 'int':
        case 'mediumint':
        case 'tinyint':
        case 'bigint':
          $column_placeholder = '%d';
          break;
        case 'float':
          $column_placeholder = '%f';
          break;
        default:
          $column_placeholder = "'%s'";
      }
      $query->add_where($table .'.'. $main_column['column'] .' = '. $column_placeholder, $arg);
      break;

    case 'link':
      $item = array();
      foreach ($db_info['columns'] as $column => $attributes) {
        $view_column_name = $attributes['column'];
        $item[$column] = $query->$view_column_name;
      }

      return l(content_format($field, $item, 'plain'), $arg .'/'. $query->$main_column['column'], array(), NULL, NULL, FALSE, TRUE);

    case 'title':
      $item = array(key($db_info['columns']) => $query);

      return content_format($field, $item, 'plain');
  }
}
