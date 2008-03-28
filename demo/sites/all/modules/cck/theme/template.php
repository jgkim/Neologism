<?php
// $Id: template.php,v 1.3.2.4 2007/03/05 22:19:04 yched Exp $

function phptemplate_field(&$node, &$field, &$items, $teaser, $page) {
  $field_empty = TRUE;
  foreach ($items as $delta => $item) {
    if (!empty($item['view']) || $item['view'] === "0") {
      $field_empty = FALSE;
      break;
    }
  }

  $variables = array(
    'node' => $node,
    'field' => $field,
    'field_type' => $field['type'],
    'field_name' => $field['field_name'],
    'field_type_css' => strtr($field['type'], '_', '-'),
    'field_name_css' => strtr($field['field_name'], '_', '-'),
    'label' => t($field['widget']['label']),
    'label_display' => isset($field['display_settings']['label']['format']) ? $field['display_settings']['label']['format'] : 'above',
    'field_empty' => $field_empty,
    'items' => $items,
    'teaser' => $teaser,
    'page' => $page,
  );

  return _phptemplate_callback('field', $variables, array('field-'. $field['field_name']));
}