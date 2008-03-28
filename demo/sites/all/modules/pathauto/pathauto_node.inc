<?php
// $Id: pathauto_node.inc,v 1.29.4.25 2008/02/03 12:31:32 greggles Exp $

/*
 * Implementation of hook_pathauto()
 */
function node_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'node';
      $settings['token_type'] = 'node';
      $settings['groupheader'] = t('Node path settings');
      $settings['patterndescr'] = t('Default path pattern (applies to all node types with blank patterns below)');
      $settings['patterndefault'] = t('content/[title-raw]');
      $settings['bulkname'] = t('Bulk generate aliases for nodes that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing nodes which do not already have aliases.');

      $patterns = token_get_list('node');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['supportsfeeds'] = 'feed';
      $nodetypes = node_get_types();
      foreach ($nodetypes as $ntype => $nodetype) {
        $fieldlabel = t('Pattern for all @node_type paths', array('@node_type' => $nodetype->name));
        $settings['patternitems'][$ntype] = $fieldlabel;
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all nodes without aliases. 
 */
function node_pathauto_bulkupdate() {

  // From all node types, only attempt to update those with patterns
  $pattern_types = array();
  $type_where = '';
  foreach (node_get_types() as $type => $info) {
    $pattern = '';
    $pattern = variable_get('pathauto_node_'. $type .'_pattern', '');

    // If it's not set, check the default
    if (!trim($pattern)) {
      $pattern = variable_get('pathauto_node_pattern', '');
    }
    if (trim($pattern)) {
      $pattern_types[] = $type;
      if (!trim($type_where)) {
        $type_where = " AND (type = '%s' ";
      }
      else {
        $type_where .= " OR type = '%s'";
      }
    }
  }
  $type_where .= ')';

  $query = "SELECT nid, type, title, uid, created, src, dst, vid FROM {node} LEFT JOIN {url_alias} ON CONCAT('node/', nid) = src WHERE src IS NULL ". $type_where;
  $result = db_query_range($query, $pattern_types, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($node_ref = db_fetch_object($result)) {
    $node = node_load($node_ref->nid, NULL, TRUE);
    $node->src = $node_ref->src;
    $node->dst = $node_ref->dst;
    if (module_exists('taxonomy')) {
        // Must populate the terms for the node here for the category
        // placeholders to work
        $node->taxonomy = array_keys(taxonomy_node_get_terms($node->nid));
    }
    $placeholders = pathauto_get_placeholders('node', $node);
    $src = "node/$node->nid";
    if ($alias = pathauto_create_alias('node', 'bulkupdate', $placeholders, $src, $node->nid, $node->type)) {
      $count++;
    }
  }

  drupal_set_message(format_plural($count,
    "Bulk generation of nodes completed, one alias generated.",
    "Bulk generation of nodes completed, @count aliases generated."));
}
