<?php

/**
 * Contruct a tree structure
 * @return json with the tree structure 
 */
function evocwidget_dynamic_get_full_classes_tree() {
  $nodes = array();
  
  $node = $_POST['node'];
  $nodes = array();
  $current_values = explode(',', $_POST['arrayOfValues']);
  
  if ( $node == 'root' ) {

    $classes = db_query(db_rewrite_sql("SELECT prefix, id, comment FROM {evoc_rdf_classes}"));
    
    $root_superclasses = array();
    while ($class = db_fetch_object($classes)) {
      $class->prefix = trim($class->prefix);
      $class->id = trim($class->id);
      $root_superclasses = evocwidget_dynamic_get_root_superclasses($class->prefix.':'.$class->id);
    }
  
    foreach ($root_superclasses as $class) {
      $leaf = (count($children_of_children = evocwidget_dynamic_gateway_get_children($class)) == 0 );
      $checked = FALSE;
      if (!empty($current_values) ) {
        if ( in_array($class, $current_values) ) {
          $checked = TRUE;
        } 
      }
      
      $nodes[] = array('text' => $class, 'id' => $class, 'leaf' => $leaf, 'iconCls' => 'class-samevoc', 'checked' => $checked, 'qtip' => 'there is no information available yet...');
    }
  }
  else {
    $children = evocwidget_dynamic_gateway_get_children($node);
    foreach( $children as $child ) {
        $class_qname = $child['prefix'].':'.$child['id'];
        $leaf = (count($children_of_children = evocwidget_dynamic_gateway_get_children($class_qname)) == 0 );
        
        $checked = FALSE;
        if (!empty($current_values) ) {
          if ( in_array($class_qname, $current_values) ) {
            $checked = TRUE;
          } 
        }
     
        $nodes[] = array('text' => $class_qname, 'id' => $class_qname, 'leaf' => $leaf, 'iconCls' => 'class-samevoc', 'checked' => $checked);
    }
  }

  drupal_json($nodes);
}

function evocwidget_dynamic_get_root_superclasses($class){
  static $root_superclasses = array();
  
  $term_qname_parts = explode(':', $class);
  $prefix = $term_qname_parts[0];
  $id = $term_qname_parts[1];
  
  $superclass = db_query(db_rewrite_sql("SELECT superclass FROM {evoc_rdf_classes} where prefix = '%s' and id = '%s'"), $prefix, $id);
  // to check if there is some super class because Drupal team remove db_num_rows function in version 6
  // and I need to now if in the query are some result and I can not do a sigle query with Count(*)
  $has_superclass = false;
  while ( $term = db_fetch_object($superclass) ) {
    $has_superclass = true;
    $term->superclass = trim($term->superclass);
    if( $term->superclass == '' ) {
      if( !_evocwidget_dynamic_gateway_in_array($class, $root_superclasses) ) {
        $root_superclasses[] = $class;  
      }
    }
    else {
      $root_superclasses = evocwidget_dynamic_get_root_superclasses($term->superclass);  
    }
  }
  
  if( !$has_superclass ) {
    if( !_evocwidget_dynamic_gateway_in_array($class, $root_superclasses) ) {
        $root_superclasses[] = $class;  
      }
  }
  
  return $root_superclasses;
}

function _evocwidget_dynamic_gateway_in_array($strclass, array $strarray_values) {
  foreach ($strarray_values as $str) {
    if( $str == $strclass ) {
      return true;
    }
  }
  
  return false; 
}

/**
 * This function search for the children of a node on the tree. As the rearch at this scope is global
 * we need to search in all the sistem including evoc_rdf_classes table.
 * @param object $node
 * @return array of children belonging to $node 
 */
function evocwidget_dynamic_gateway_get_children($node) {
  $children = db_query(db_rewrite_sql("select n.title, n.nid from content_field_superclass2 as c inner join node as n on c.nid = n.nid 
    where c.field_superclass2_evoc_term = '%s'"), $node);
   
  // get children from vocabularies on Drupal content 
  $arr_children = array();  
  while ($child = db_fetch_object($children)) {
    $classes = db_query(db_rewrite_sql("select e.prefix, e.superclass from {evoc_rdf_classes} as e where e.id = '%s'"), $child->title);
    // may there is more than one class with the same title but belong to a different vocabulary
    while ($class = db_fetch_object($classes)) {
      $class->prefix = trim($class->prefix);
      $class->superclass = trim($class->superclass); 
      // check if the current $class is well selected, because might be a class with same title/id and different vocabulary/prefix
      // otherwise we need to join more tables to make the correct query.
      // this fix when there is two class with same name but different prefix, eg: eg:Agent and foaf:Agent
      if( $class->superclass == $node ) {
        $arr = array('prefix' => $class->prefix, 'id' => $child->title);
        if( !in_array($arr, $arr_children) ) {
          $arr_children[] = $arr; 
        }
      }
    }
  }
  
  // get child from evoc classes
  $children = db_query(db_rewrite_sql("select e.prefix, e.id from {evoc_rdf_classes} as e where e.superclass = '%s'"), $node);
  while ($child = db_fetch_object($children)) {
    $child->prefix = trim($child->prefix);
    $child->id = trim($child->id);
    // may there is more than one class with the same title but belong to a different vocabulary
    $arr = array('prefix' => $child->prefix, 'id' => $child->id);
    if( !in_array($arr, $arr_children) ) {
      $arr_children[] = $arr; 
    }
  }
  
  return $arr_children;
}
