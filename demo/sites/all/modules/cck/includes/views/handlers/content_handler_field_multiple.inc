<?php
// $Id: content_handler_field_multiple.inc,v 1.1.2.1 2008/09/04 18:12:27 yched Exp $

/**
 * An extended subclass for field handling that adds multiple field grouping.
 *
 * Fields that want multiple value grouping options in addition to basic
 * field and formatter handling can extend this class.
 */
class content_handler_field_multiple extends content_handler_field {
  var $defer_query;

  function options(&$options) {
    parent::options($options);
    $field = $this->content_field;
    $options['multiple'] = array(
      'group' => TRUE,
      'multiple_number' => '',
      'multiple_from' => '',
      'multiple_reversed' => FALSE,
    );
  }

  /**
   * Provide 'group multiple values' option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $field = $this->content_field;
    $options = $this->options;

    $form['multiple'] = array(
      '#access' => $field['multiple'],
    );
    $form['multiple']['group'] = array(
      '#title' => t('Group multiple values'),
      '#type' => 'checkbox',
      '#default_value' => $options['multiple']['group'],
    );
    // Make the string translatable by keeping it as a whole rather than
    // translating prefix and suffix separately.
    list($prefix, $suffix) = explode('@count', t('Show @count value(s)'));
    $form['multiple']['multiple_number'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#field_prefix' => $prefix,
      '#field_suffix' => $suffix,
      '#default_value' => $options['multiple']['multiple_number'],
      '#prefix' => '<div class="container-inline">',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
    );
    list($prefix, $suffix) = explode('@count', t('starting from @count'));
    $form['multiple']['multiple_from'] = array(
      '#type' => 'textfield',
      '#size' => 5,
      '#field_prefix' => $prefix,
      '#field_suffix' => $suffix,
      '#default_value' => $options['multiple']['multiple_from'],
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
    );
    $form['multiple']['multiple_reversed'] = array(
      '#title' => t('Reversed (start from last values)'),
      '#type' => 'checkbox',
      '#default_value' => $options['multiple']['multiple_reversed'],
      '#suffix' => '</div>',
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-multiple-group' => array(TRUE)),
    );
  }

  function query() {
    $field = $this->content_field;
    $options = $this->options;

    // 'Group multiple values' : the values for the field will be retrieved
    // in a separate query (one per field).
    $this->defer_query = $options['multiple']['group'] && $field['multiple'];

    // If this is not a grouped field, use the generic field handler query.
    if (!$this->defer_query) {
      return parent::query();
    }

    // We don't add anything to the query. The data is retrieved in pre_render()
    // in order to avoid duplicate results.
    $this->field_alias = $this->real_field;

    // If the field is sortable (table sort), we just have to join the table
    // ("Node: Distinct" will be required to avoid duplicates...)
    // if ($field['sortable']) {
    //   $this->ensure_my_table();
    // }
  }

  function pre_render($values) {
    // There are no values to render in a summary view.
    if (isset($this->view->build_info['summary'])) {
      return parent::pre_render($values);
    }
    // If this is not a grouped field, use the parent pre_render().
    if (!$this->defer_query || empty($values)) {
      return parent::pre_render($values);
    }

    $field = $this->content_field;
    $db_info = content_database_info($field);
    $options = $this->options;

    $this->field_values = array();
    // Build the list of nids to retrieve.
    // TODO: try fetching from cache_content first ??
    $nids = array();
    $this->field_values = array();
    foreach ($values as $value) {
      $nid = $value->{$this->additional_fields['nid']};
      $nids[] = $nid;
      $this->field_values[$nid] = array();
    }

    // List columns to retrieve.
    $table_alias = content_views_tablename($field);
    $query_columns = array("$table_alias.delta AS delta");
    foreach ($db_info['columns'] as $column => $attributes) {
      $query_columns[] = "$table_alias.$attributes[column] AS $column";
    }
    // Make sure node.nid is aliased so it isn't confused with
    // nodereference's nid column.
    $query = "SELECT node.nid AS parent_nid, ". implode(', ', $query_columns) .
             " FROM {node} node".
             " LEFT JOIN {". $db_info['table'] ."} $table_alias ON node.vid = $table_alias.vid".
             " WHERE node.nid IN (". implode(',', $nids) .")".
             " ORDER BY node.nid ASC, $table_alias.delta ". ($options['multiple']['multiple_reversed'] ? 'DESC' : 'ASC');
    $result = db_query($query);

    while ($item = db_fetch_array($result)) {
      $delta = !empty($item['delta']) ? $item['delta'] : 0;
      $item['#delta'] = $item['delta'];
      unset($item['delta']);
      $this->field_values[$item['parent_nid']][$delta] = $item;
    }
  }

  function render($values) {
    // If this is not a grouped field, use content_handler_field::render().
    if (!$this->defer_query || empty($this->field_values)) {
      return parent::render($values);
    }

    $field = $this->content_field;
    $options = $this->options;

    // $values will be used as a fake $node object;
    // we provide a build_mode for rendering.
    // TODO: we can stick any value in there -
    // what would make most sense ?  row_style ?
    $values->build_mode = 'views';

    // This needs to be set for the $this->render_link() to work. It would
    // have been set in the query, if we hadn't bypassed the normal query.
    // TODO there may be a better way to do this.
    $this->aliases['nid'] = 'nid';

    $count = 0;
    $items = array();
    foreach ($this->field_values[$values->{$this->additional_fields['nid']}] as $item) {
      // 'Starting from n': Let the first n values pass.
      if (empty($options['multiple']['multiple_from']) || ($count >= $options['multiple']['multiple_from'])) {
        // 'Display m values'.
        if (empty($options['multiple']['multiple_number']) || (count($items) < $options['multiple']['multiple_number'])) {
          $value = content_format($field, $item, $options['format'], $values);
          if (!empty($value)) {
            $items[] = $this->render_link($value, $values);
          }
        }
        else {
          // We displayed the number of values that were asked, no need to render more.
          break;
        }
      }
      $count++;
    }
    if (count($items) > 1) {
      // TODO: could we use generic field display ?
      return theme('content_view_multiple_field', $items, $field, $values);
    }
    elseif (count($items) == 1) {
      return $items[0];
    }
    else {
      return '';
    }
  }
}