<?php
// $Id: content_pathauto.inc,v 1.1 2006/08/20 18:28:11 JonBob Exp $

/**
 * @file
 * Interface between content.module and pathauto.module.
 */

/*
 * Implementation of hook_pathauto_node().
 *
 * Allows users to use the textual representation of any field as a component
 * of paths.
 */
function content_pathauto_node($op, $node = NULL) {
  switch ($op) {
    case 'placeholders':
      $placeholders = array();

      foreach (content_fields() as $field) {
        $placeholders['['. $field['field_name'] .']'] = t($field['widget']['label']);
      }
      return $placeholders;

    case 'values':
      $results = array();

      // Get node output (filtered and with module-specific fields).
      if (node_hook($node, 'view')) {
        node_invoke($node, 'view', false, false);
      }
      else {
        $node = node_prepare($node, false);
      }
      // Allow modules to change $node->body before viewing.
      node_invoke_nodeapi($node, 'view', false, false);
      // Get node output (filtered and with module-specific fields).
      if (node_hook($node, 'view')) {
        node_invoke($node, 'view', false, false);
      }
      else {
        $node = node_prepare($node, false);
      }
      // Allow modules to change $node->body before viewing.
      node_invoke_nodeapi($node, 'view', false, false);

      foreach (content_fields() as $field) {
        $items = $node->$field['field_name'];
        $results['['. $field['field_name'] .']'] = pathauto_cleanstring(strip_tags($items[0]['view']));
      }

      return $results;
  }
}
