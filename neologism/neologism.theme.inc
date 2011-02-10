<?php

/**
 * This file contains Neologism's theming functions.
 */

/**
 * Preprocessor for rendering class and property nodes via the node-neo_term
 * template. Used on vocabulary overview pages.
 */
function neologism_preprocess_node(&$variables) {
  if ($variables['type'] != NEOLOGISM_CT_CLASS && $variables['type'] != NEOLOGISM_CT_PROPERTY) return;
  $variables['is_class'] = $variables['type'] == NEOLOGISM_CT_CLASS;
  $variables['template_file'] = 'node-neo_term';
  $variables['term_uri'] = $variables['vocabulary']->namespace_uri . $variables['title'];
  $variables['qname'] = $variables['vocabulary']->prefix . ':' . $variables['title'];
  $variables['term_label'] = $variables['field_label'][0]['value'];
  $variables['term_comment'] = $variables['field_comment'][0]['value'];
  $variables['has_custom_namespace'] = !empty($variables['vocabulary']->field_custom_namespace[0]['value']);
  $variables['original_body'] = check_markup($variables['original_body'], $node->format, FALSE);
  $variables['superclasses'] = _neologism_themed_qnames($variables['field_superclass2']);
  $variables['disjoints'] = _neologism_themed_qnames($variables['field_disjointwith2']);
  $variables['domains'] = _neologism_themed_qnames($variables['field_domain2']);
  $variables['ranges'] = _neologism_themed_qnames($variables['field_range2']);
  $variables['inverses'] = _neologism_themed_qnames($variables['field_inverse2']);
  $variables['superproperties'] = _neologism_themed_qnames($variables['field_superproperty2']);
  $variables['types'] = array();
  if (!empty($variables['field_fp'][0]['value'])) {
    $variables['types'][] = trim(theme('neologism_qname', 'owl:FunctionalProperty'));
  }
  if (!empty($variables['field_ifp'][0]['value'])) {
    $variables['types'][] = trim(theme('neologism_qname', 'owl:InverseFunctionalProperty'));
  }
  if ($variables['page']) {
    drupal_set_title($variables['qname']);
    drupal_set_breadcrumb(_neologism_build_breadcrumb($variables['vocabulary']->path, $variables['vocabulary']->title));
  }
}

/**
 * Hack to make the View and Edit tabs on class/property edit forms disappear.
 * We don't want to give access to the View tab because classes and properties
 * are not supposed to have their own pages.
 */
function neologism_preprocess_page(&$variables) {
  if ($variables['node']->type == NEOLOGISM_CT_CLASS || $variables['node']->type == NEOLOGISM_CT_PROPERTY) {
    $variables['tabs'] = null;
  }
}

/**
 * Helper function.
 */
function _neologism_themed_qnames($field) {
  $result = array();
  if (!is_array($field)) return;
  foreach ($field as $term) {
    if (empty($term['evoc_term'])) continue;
    $result[] = trim(theme('neologism_qname', $term['evoc_term']));
  }
  return $result;
}

/**
 * Themes a QName by turning it into a clickable link to the expanded URI.
 *
 * @param $qname
 *   A QName, such as foaf:Person.
 */
function theme_neologism_qname($qname) {
  return l($qname, rdf_qname_to_uri($qname));
}
