<?php

/**
 * Construct a tree structure for an specific vocabulary. This' the gateway for the treeview
 * @return json with the tree structure 
 */
function neologism_gateway_get_classes_tree() {
  $voc['id'] = $_POST['voc_id'];
  $voc['title'] = $_POST['voc_title'];
  $node = $_POST['node'];
  $store = array();
  $nodes = array();
  if ( $node == 'super' ) {
    $classes = db_query(db_rewrite_sql('select * from {evoc_rdf_classes} where prefix = "'.$voc['title'].'"'));  
    
    while ( $class = db_fetch_object($classes) ) {
      $qname = $class->prefix.':'.$class->id;
      $store[$qname] = null;
      $store[$qname]['comment'] = $class->comment;
      $store[$qname]['label'] = $class->label;
      if ( $class->superclasses > 0 ) {
      	$superclasses = db_query('select superclass from {evoc_rdf_superclasses} where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'"');
      	while ( $object = db_fetch_object($superclasses) ) {
      		$store[$object->superclass]['subclasses'][] = $qname; 
      		$store[$qname]['rdfs:subClassOf'][] = $object->superclass; 
      	}
      }
    }
    
    foreach ($store as $key => $val ) {  
    	if ( !isset($val['rdfs:subClassOf']) ) {
  			$nodes[] = _neologism_buildSubclassesTreeInOrder($store, $key, $voc['title']);
    	}
	  }
  }

  drupal_json($nodes);
}

function &array_rpop(&$a){
    end($a);
    $k=key($a);
    $v=&$a[$k];
    unset($a[$k]);
    return $v;
}

function _neologism_buildSubclassesTreeInOrder(array &$store, $class, $vocabulary) {
	$stack = array();
	$nodes = array();
	$stack[] = array($class, &$nodes);
	
	while( count($stack) ) {
		$arr = &array_rpop($stack);
		$current = $arr[0]; 
		$node = &$arr[1]; 
		
		$term_qname_parts = explode(':', $current);
  	$prefix = $term_qname_parts[0];
  	$id = $term_qname_parts[1];
  	
  	$sameVocabulary = ($prefix == $vocabulary);
  	$qtip = '<b>'.$store[$current]['label'].'</b><br/>'.$store[$current]['comment'];
		
		if( count($store[$current]['subclasses']) ) {
			if( $node ) {
				$node['leaf'] = false;
				$node['children'][] = array(
					'text' => $current,
					'id' => $current,
					'children' => null,
					'leaf' => true,
					'iconCls' => $sameVocabulary ? 'class-samevoc' : 'class-diffvoc',
					'cls' => $sameVocabulary ? 'currentvoc' : '',
					'qtip' => $qtip,
				);
				
				$node = &$node['children'][count($node['children'])-1];
			}
			else {
				$node = array(
					'text' => $current,
					'id' => $current,
					'children' => null,
					'leaf' => true,
					'iconCls' => $sameVocabulary ? 'class-samevoc' : 'class-diffvoc',
					'cls' => $sameVocabulary ? 'currentvoc' : '',
					'qtip' => $qtip,
				);
			}
			
			foreach( $store[$current]['subclasses'] as $key => $val ) {
				$stack[] = array($val, &$node);
			}
			
			continue;
		}
		
		if( $node ) {
			$node['leaf'] = false;
			$node['children'][] = array(
				'text' => $current,
				'id' => $current,
				'children' => null,
				'leaf' => true,
				'iconCls' => $sameVocabulary ? 'class-samevoc' : 'class-diffvoc',
				'cls' => $sameVocabulary ? 'currentvoc' : '',
				'qtip' => $qtip,
			);
			
			$node = &$node['children'][count($node['children'])-1];
		}
		else {
			$node = array(
				'text' => $current,
				'id' => $current,
				'children' => null,
				'leaf' => true,
				'iconCls' => $sameVocabulary ? 'class-samevoc' : 'class-diffvoc',
				'cls' => $sameVocabulary ? 'currentvoc' : '',
				'qtip' => $qtip,
			);
		}
	}
	
	/*
	'qtip' => $qtip,
          	//'disjointwith' => $disjointwith,
          	//'superclasses' => $superclasses,
          	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
	*/
	return $nodes;
}

/**
 * This recurive function search for chindren of $node return class from the same $voc. 
 * If the parent does not belong to the $voc but has children that does, this parent is added as well.
 * @param object $node
 * @param object $voc
 * @param object $add_checkbox [optional]
 * @return 
 */
function neologism_gateway_get_class_children($node, $voc = NULL, $add_checkbox = FALSE, array &$disjointwith_array = NULL) {
  $nodes = array();
  
  static $array_of_id = array();
  
  $children = db_query('select prefix, reference from {evoc_rdf_superclasses} where superclass = "'.$node.'"');
    
  while ($child = db_fetch_object($children)) {
    $class = db_fetch_object(db_query('select * from evoc_rdf_classes where prefix = "'.$child->prefix.'" and id = "'.$child->reference.'" '));
    if ( $class ) {
      $class->prefix = trim($class->prefix);
      $class->id = trim($class->id); 
      $qname = $class->prefix.':'.$class->id;
      
      // extra information needed by the treeview
      $extra_information = true;
      $realId = '';
      if( isset($array_of_id[$qname]) ) {
      	$modified_id = $array_of_id[$qname].'_'.$qname;
      	$array_of_id[$qname] += 1;
      	$realId = $qname;
      }
      else {
      	$array_of_id[$qname] = 1;	
      	$extra_information = false;
      }
      
      //-----------------------------------------
      // fetch extra information
      
      // fetch the disjointwith
      $disjointwith = array();
      if( $class->ndisjointwith > 0 ) {
				$result = db_query('select disjointwith from {evoc_rdf_disjointwith} where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'" ');
				while( $c = db_fetch_object($result) ) {
					$disjointwith[] = $c->disjointwith;
				}
				
				if( isset($disjointwith_array) && is_array($disjointwith_array) ) {
					$disjointwith_array[$qname] = $disjointwith;
				}
      }
      
      // fetch the superclasses
    	$superclasses = array();
      if( $class->superclasses > 0 ) {
				$result = db_query('select superclass from {evoc_rdf_superclasses} where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'" ');
				while( $s = db_fetch_object($result) ) {
					$superclasses[] = $s->superclass;
				}
      }
      
      $children_nodes = neologism_gateway_get_class_children($qname, $voc, $add_checkbox, $disjointwith_array);  
      if( $voc ) {
        if( $class->prefix == $voc || _neologism_gateway_in_nodes($voc, $children_nodes) ) {
          $leaf = count($children_nodes) == 0;
          $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
          $nodes[] = array(
            'text' => $qname, 
            'id' => (!$extra_information) ? $qname : $modified_id, 
            'leaf' => $leaf, 
            'iconCls' => ($class->prefix == $voc) ? 'class-samevoc' : 'class-diffvoc', 
            'cls' => ($class->prefix == $voc) ? 'currentvoc' : '',            
            'children' => $children_nodes, 
            'qtip' => $qtip,
          	'disjointwith' => $disjointwith,
          	'superclasses' => $superclasses,
          	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
          );
          
          if( $extra_information ) {
          	$nodes[count($nodes)-1]['realid'] = $qname; 
          } 
          
          if( $add_checkbox ) {
            $nodes[count($nodes)-1]['checked'] = false;
          }
        }
      } else {
        $leaf = count($children_nodes) == 0;
        $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
        $nodes[] = array(
          'text' => $qname, 
          'id' => (!$extra_information) ? $qname : $modified_id, 
          'leaf' => $leaf, 
          'iconCls' => 'class-samevoc', 
          'children' => $children_nodes, 
          'qtip' => $qtip,
        	'disjointwith' => $disjointwith,
        	'superclasses' => $superclasses,
        	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
        );
        
      	if( $extra_information ) {
        	$nodes[count($nodes)-1]['realid'] = $qname; 
        }
          
        if( $add_checkbox ) {
          $nodes[count($nodes)-1]['checked'] = false;
        }  
      }
      
    }
  }
  
  return $nodes;
}


//-----------------------------------------------------------------------------------------------------------------
// functions for objectproperty_tree
function neologism_gateway_get_properties_tree() {
  $voc['id'] = $_POST['voc_id'];
  $voc['title'] = $_POST['voc_title'];
  
  $node = $_POST['node'];
  $nodes = array();
  
  if ( $node == 'super' ) {
    $properties = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_properties} where superproperties = "0"'));

    while ($property = db_fetch_object($properties)) {
      $qname = $property->prefix.':'.$property->id;
      $children = neologism_gateway_get_property_children($qname, $voc['title']);
      if( $property->prefix == $voc['title'] || _neologism_gateway_in_nodes($voc['title'], $children) ) {
        $qtip = '<b>'.$property->label.'</b><br/>'.$property->comment;
        $leaf = count($children) == 0;
        $nodes[] = array(
          'text' => $qname, 
          'id' => $qname, 
          'leaf' => $leaf, 
          'iconCls' => ($property->prefix == $voc['title']) ? 'property-samevoc' : 'property-diffvoc',
          'cls' => ($property->prefix == $voc['title']) ? 'currentvoc' : '', 
          'children' => $children, 
          'qtip' => $qtip
        );        
      }
    }
  }
  
  drupal_json($nodes);
}

function neologism_gateway_get_root_superproperties($property) {
  static $root_superproperties = array();
  
  $term_qname_parts = explode(':', $property);
  $prefix = $term_qname_parts[0];
  $id = $term_qname_parts[1];
  
  $object = db_fetch_object(db_query(db_rewrite_sql("select superproperties from {evoc_rdf_properties} where prefix = '%s' and id = '%s'"), $prefix, $id));
  if ( $object->superproperties > 0 ) {
    $superproperty = db_query(db_rewrite_sql("SELECT superproperty FROM {evoc_rdf_superproperties} where prefix = '%s' and reference = '%s'"), $prefix, $id);
    while ( $term = db_fetch_object($superproperty) ) {
      $term->superproperty = trim($term->superproperty);
      $root_superproperties = neologism_gateway_get_root_superclasses($term->superproperty);  
    }
  }
  else {
    if( !_neologism_gateway_in_array($property, $root_superproperties) ) {
      $root_superproperties[] = $property;  
    }
  }
  
  return $root_superproperties;
}


function neologism_gateway_get_property_children($node, $voc = NULL, $add_checkbox = FALSE) {
  $nodes = array();
  
  $children = db_query('select prefix, reference from {evoc_rdf_superproperties} where superproperty = "'.$node.'"');
    
  while ($child = db_fetch_object($children)) {
    $property = db_fetch_object(db_query('select * from evoc_rdf_properties where prefix = "'.$child->prefix.'" and id = "'.$child->reference.'" '));
    if ( $property ) {
      $property->prefix = trim($property->prefix);
      $property->id = trim($property->id); 
      $qname = $property->prefix.':'.$property->id;
      
    	$domain = array();
      if( $property->domains > 0 ) {
      	$domains = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_propertiesdomains} WHERE prefix = "%s" AND reference = "%s"'), $property->prefix, $property->id);
      	while ($obj = db_fetch_object($domains)) {
      		$domain[]	= $obj->rdf_domain;
      	}
      }
      
    	$range = array();
      if( $property->ranges > 0 ) {
      	$ranges = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_propertiesranges} WHERE prefix = "%s" AND reference = "%s"'), $property->prefix, $property->id);
      	while ($obj = db_fetch_object($ranges)) {
      		$range[]	= $obj->rdf_range;
      	}
      }
      
      $children_nodes = neologism_gateway_get_property_children($qname, $voc, $add_checkbox);  
      if( $voc ) {
        if( $property->prefix == $voc || _neologism_gateway_in_nodes($voc, $children_nodes) ) {
          $leaf = count($children_nodes) == 0;
          $qtip = '<b>'.$property->label.'</b><br/>'.$property->comment;
          $nodes[] = array(
            'text' => $qname, 
            'id' => $qname, 
            'leaf' => $leaf, 
            'iconCls' => ($property->prefix == $voc) ? 'property-samevoc' : 'property-diffvoc', 
            'cls' => ($property->prefix == $voc) ? 'currentvoc' : '',
            'children' => $children_nodes, 
            'qtip' => $qtip,
          	'domain' => $domain,
      			'range' => $range	
          );
          
          if( $add_checkbox ) {
            $nodes[count($nodes)-1]['checked'] = false;
          }
        }
      } else {
        $leaf = count($children_nodes) == 0;
        $qtip = '<b>'.$property->label.'</b><br/>'.$property->comment;
        $nodes[] = array(
          'text' => $qname, 
          'id' => $qname, 
          'leaf' => $leaf, 
          'iconCls' => 'property-samevoc', 
          'children' => $children_nodes, 
          'qtip' => $qtip,
	        'domain' => $domain,
	      	'range' => $range
        );
        
        if( $add_checkbox ) {
          $nodes[count($nodes)-1]['checked'] = false;
        }
      }
    }
  }
  
  return $nodes;
}

//-----------------------------------------------------------------------------------------------------------
// These functions are more completed that the above. I planned to fix the function above for tunning process
//

/**
 * Construct the tree structure for a Tree using ExtJS Tree structure
 * @return json with the tree structure 
 */
function neologism_gateway_get_full_classes_tree() {
  $nodes = array();
  $node = $_REQUEST['node'];
  
  // TODO we could pass as a parameter when to use the function to infer the disjointwith
  $infer_disjoint = TRUE;
  static $disjointwith_array = array();
    
  if ( $node == 'root' ) {
    $classes = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_classes} where superclasses = "0"'));

    while ($class = db_fetch_object($classes)) {
      $qname = $class->prefix.':'.$class->id;
      
    	// fetch the disjointwith for root classes
      $disjointwith = array();
      if( $class->ndisjointwith > 0 ) {
				$result = db_query('select disjointwith from evoc_rdf_disjointwith where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'" ');
				while( $c = db_fetch_object($result) ) {
					$disjointwith[] = $c->disjointwith;
				}
				
				// add disjointwith to the main array to use it after build the nodes
				$disjointwith_array[$qname] = $disjointwith;
      }
      
      $children = neologism_gateway_get_class_children($qname, NULL, TRUE, $disjointwith_array);
      $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
      $leaf = count($children) == 0;
      $nodes[] = array(
        'text' => $qname, 
        'id' => $qname, 
        'leaf' => $leaf, 
        'iconCls' => 'class-samevoc', 
        'children' => $children, 
        'checked' => false,
        'qtip' => $qtip,
      	'disjointwith' => $disjointwith,
      	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
      );        
    }
  
  
	  // TODO At this point should exists an array with all the disjointwith classes and 
	  // we execute some functio that works as a rasoner to infer all the posibles disjointwith 
	  // between clases
	  infer_disjointwith($nodes, $disjointwith_array);
  }

  drupal_json($nodes);
}

/**
 * Check for the known disjointwith to infer unknown disjointwith, cause
 * this is a simetric property
 * @param unknown_type $nodes
 * @param array $disjointwith_array
 * @return unknown_type
 */
function infer_disjointwith(&$nodes, array $disjointwith_array, array $parent_disjointwith = NULL) {
	for ($i = 0; $i < count($nodes); $i++ ) {
		
		if( $parent_disjointwith != NULL ) {
			$nodes[$i]['disjointwith'] = array_merge($parent_disjointwith, $nodes[$i]['disjointwith']);	
		}
		
		foreach( $disjointwith_array as $key => $array_values ) {
			foreach( $array_values as $value ) {
				if( $nodes[$i]['text'] == $value && !in_array($key, $nodes[$i]['disjointwith']) )	{
					$nodes[$i]['disjointwith'][] = $key;	
				}
			}
		}
		
		if( count($nodes[$i]['children']) > 0 ) {
			infer_disjointwith($nodes[$i]['children'], $disjointwith_array, $nodes[$i]['disjointwith']);
		}
	}
}

/**
 * 
 * @param $class
 * @return unknown_type
 */
function neologism_gateway_get_root_superclasses($class) {
 
  static $root_superclasses = array();
  
  $term_qname_parts = explode(':', $class);
  $prefix = $term_qname_parts[0];
  $id = $term_qname_parts[1];
  
  $object = db_fetch_object(db_query(db_rewrite_sql("select superclasses from {evoc_rdf_classes} where prefix = '%s' and id = '%s'"), $prefix, $id));
  if ( $object->superclasses > 0 ) {
    $superclass = db_query(db_rewrite_sql("SELECT superclass FROM {evoc_rdf_superclasses} where prefix = '%s' and reference = '%s'"), $prefix, $id);
    while ( $term = db_fetch_object($superclass) ) {
      $term->superclass = trim($term->superclass);
      $root_superclasses = neologism_gateway_get_root_superclasses($term->superclass);  
    }
  }
  else {
    if( !_neologism_gateway_in_array($class, $root_superclasses) ) {
      $root_superclasses[] = $class;  
    }
  }
  
  return $root_superclasses;
}

/**
 * This recursive function return all the children from $node
 * @param object $node
 * @return 
 */
/*
function neologism_gateway_get_children($node, $reset = false) {
  $nodes = array();
  
  // search for the children in all the tables
  $children = db_query(db_rewrite_sql("select n.title, n.nid from 
    content_field_superclass2 as c inner join node as n on c.nid = n.nid 
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
  
  // at this point we are finished to search all children of current node.
  // now  we need to expand the children
  
  
  // iterate through the children
  foreach( $arr_children as $child ) {
    $class_qname = $child['prefix'].':'.$child['id'];
    $children = neologism_gateway_get_children($class_qname);
    $leaf = count($children) == 0;
    $nodes[] = array(
      'text' => $class_qname, 
      'id' => $class_qname, 
      'leaf' => $leaf, 
      'iconCls' => 'class-samevoc', 
      'children' => $children, 
      'checked' => false
    ); 
  }
  
  return $nodes;
}
*/

// properties

/**
 * Construct the tree structure for a Tree using ExtJS Tree structure
 * @return json with the tree structure 
 */
function neologism_gateway_get_full_properties_tree() {
  $nodes = array();
  $node = $_REQUEST['node'];
    
  if ( $node == 'root' ) {
    $properties = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_properties} where superproperties = "0"'));
    
    while ($property = db_fetch_object($properties)) {
      $qname = $property->prefix.':'.$property->id;
      
      $domain = array();
      if( $property->domains > 0 ) {
      	$domains = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_propertiesdomains} WHERE prefix = "%s" AND reference = "%s"'), $property->prefix, $property->id);
      	while ($obj = db_fetch_object($domains)) {
      		$domain[]	= $obj->rdf_domain;
      	}
      }
      
    	$range = array();
      if( $property->ranges > 0 ) {
      	$ranges = db_query(db_rewrite_sql('SELECT * FROM {evoc_rdf_propertiesranges} WHERE prefix = "%s" AND reference = "%s"'), $property->prefix, $property->id);
      	while ($obj = db_fetch_object($ranges)) {
      		$range[]	= $obj->rdf_range;
      	}
      }
      
      $children = neologism_gateway_get_property_children($qname, NULL, TRUE);
      $qtip = '<b>'.$property->label.'</b><br/>'.$property->comment;
      $leaf = count($children) == 0;
      $nodes[] = array(
        'text' => $qname, 
        'id' => $qname, 
        'leaf' => $leaf, 
        'iconCls' => 'property-samevoc', 
        'children' => $children, 
        'checked' => false,
        'qtip' => $qtip,
      	'domain' => $domain,
      	'range' => $range
      );        
    }
  }

  drupal_json($nodes);
}

function _neologism_gateway_in_array($strproperty, array $strarray_values) {
  foreach ($strarray_values as $str) {
    if( $str == $strproperty ) {
      return true;
    }
  }
  
  return false; 
}

/**
 * Compare if in $nodes array exists some qname with prefix $prefix.
 * Using the $node['text'] attribute because the $node['id'] attr changes sometimes
 * @param unknown_type $prefix
 * @param array $nodes
 * @return unknown_type
 */
function _neologism_gateway_in_nodes($prefix, array $nodes) {
  $result = false;
  foreach ($nodes as $node) {    
  	$qterm_splited = explode(':', $node['text']);
    if( $prefix == $qterm_splited[0] ) {
      $result = true;
    } elseif( !empty($node['children']) ) {
      $result = _neologism_gateway_in_nodes($prefix, $node['children']); 
    }
  }
  return $result;  
}

//------------------------------------------------------------------------
// version old
/**
 * Construct a tree structure for an specific vocabulary. This' the gateway for the treeview
 * @return json with the tree structure 
 */
function neologism_gateway_get_classes_tree_old() {
  $voc['id'] = $_POST['voc_id'];
  $voc['title'] = $_POST['voc_title'];
  
  $node = $_POST['node'];
  $nodes = array();
  if ( $node == 'super' ) {
    $classes = db_query(db_rewrite_sql('select * from {evoc_rdf_classes} where superclasses = "0"'));  
    
    while ( $class = db_fetch_object($classes) ) {
      $qname = $class->prefix.':'.$class->id;
      $children = neologism_gateway_get_class_children($qname, $voc['title']);
      if( $class->prefix == $voc['title'] || _neologism_gateway_in_nodes($voc['title'], $children) ) {
        $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
        $leaf = count($children) == 0;
        $nodes[] = array(
          'text' => $qname, 
          'id' => $qname, 
          'leaf' => $leaf, 
          'iconCls' => ($class->prefix == $voc['title']) ? 'class-samevoc' : 'class-diffvoc',
          'cls' => ($class->prefix == $voc['title']) ? 'currentvoc' : '', 
          'children' => $children, 
          'qtip' => $qtip
        );        
      }
    }
  }

  drupal_json($nodes);
}

/**
 * This recurive function search for chindren of $node return class from the same $voc. 
 * If the parent does not belong to the $voc but has children that does, this parent is added as well.
 * @param object $node
 * @param object $voc
 * @param object $add_checkbox [optional]
 * @return 
 */
function neologism_gateway_get_class_children_old($node, $voc = NULL, $add_checkbox = FALSE, array &$disjointwith_array = NULL) {
  $nodes = array();
  
  static $array_of_id = array();
  
  $children = db_query('select prefix, reference from {evoc_rdf_superclasses} where superclass = "'.$node.'"');
    
  while ($child = db_fetch_object($children)) {
    $class = db_fetch_object(db_query('select * from evoc_rdf_classes where prefix = "'.$child->prefix.'" and id = "'.$child->reference.'" '));
    if ( $class ) {
      $class->prefix = trim($class->prefix);
      $class->id = trim($class->id); 
      $qname = $class->prefix.':'.$class->id;
      
      // extra information needed by the treeview
      $extra_information = true;
      $realId = '';
      if( isset($array_of_id[$qname]) ) {
      	$modified_id = $array_of_id[$qname].'_'.$qname;
      	$array_of_id[$qname] += 1;
      	$realId = $qname;
      }
      else {
      	$array_of_id[$qname] = 1;	
      	$extra_information = false;
      }
      
      //-----------------------------------------
      // fetch extra information
      
      // fetch the disjointwith
      $disjointwith = array();
      if( $class->ndisjointwith > 0 ) {
				$result = db_query('select disjointwith from {evoc_rdf_disjointwith} where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'" ');
				while( $c = db_fetch_object($result) ) {
					$disjointwith[] = $c->disjointwith;
				}
				
				if( isset($disjointwith_array) && is_array($disjointwith_array) ) {
					$disjointwith_array[$qname] = $disjointwith;
				}
      }
      
      // fetch the superclasses
    	$superclasses = array();
      if( $class->superclasses > 0 ) {
				$result = db_query('select superclass from {evoc_rdf_superclasses} where prefix = "'.$class->prefix.'" and reference = "'.$class->id.'" ');
				while( $s = db_fetch_object($result) ) {
					$superclasses[] = $s->superclass;
				}
      }
      
      $children_nodes = neologism_gateway_get_class_children($qname, $voc, $add_checkbox, $disjointwith_array);  
      if( $voc ) {
        if( $class->prefix == $voc || _neologism_gateway_in_nodes($voc, $children_nodes) ) {
          $leaf = count($children_nodes) == 0;
          $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
          $nodes[] = array(
            'text' => $qname, 
            'id' => (!$extra_information) ? $qname : $modified_id, 
            'leaf' => $leaf, 
            'iconCls' => ($class->prefix == $voc) ? 'class-samevoc' : 'class-diffvoc', 
            'cls' => ($class->prefix == $voc) ? 'currentvoc' : '',            
            'children' => $children_nodes, 
            'qtip' => $qtip,
          	'disjointwith' => $disjointwith,
          	'superclasses' => $superclasses,
          	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
          );
          
          if( $extra_information ) {
          	$nodes[count($nodes)-1]['realid'] = $qname; 
          } 
          
          if( $add_checkbox ) {
            $nodes[count($nodes)-1]['checked'] = false;
          }
        }
      } else {
        $leaf = count($children_nodes) == 0;
        $qtip = '<b>'.$class->label.'</b><br/>'.$class->comment;
        $nodes[] = array(
          'text' => $qname, 
          'id' => (!$extra_information) ? $qname : $modified_id, 
          'leaf' => $leaf, 
          'iconCls' => 'class-samevoc', 
          'children' => $children_nodes, 
          'qtip' => $qtip,
        	'disjointwith' => $disjointwith,
        	'superclasses' => $superclasses,
        	'nodeStatus' => 0	// send to the client this attribute that represent the  NORMAL status for a node
        );
        
      	if( $extra_information ) {
        	$nodes[count($nodes)-1]['realid'] = $qname; 
        }
          
        if( $add_checkbox ) {
          $nodes[count($nodes)-1]['checked'] = false;
        }  
      }
      
    }
  }
  
  return $nodes;
}
