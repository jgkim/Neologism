<?php

/**
 * This file contains Neologism's theming functions.
 */

/**
 * Preprocessor for rendering class and property nodes via the node-neo_term
 * template. Used on vocabulary overview pages.
 */
function neologism_preprocess_node(&$variables) {
  if ($variables['type'] == 'neo_vocabulary') {
    $variables['count_classes'] = db_result(db_query(db_rewrite_sql("SELECT COUNT(*) FROM {content_field_vocabulary} c INNER JOIN {node} n ON c.nid = n.nid WHERE c.field_vocabulary_nid = %d AND n.type = '%s'"), $variables['nid'], NEOLOGISM_CT_CLASS));
    $variables['count_properties'] = db_result(db_query(db_rewrite_sql("SELECT COUNT(*) FROM {content_field_vocabulary} c INNER JOIN {node} n ON c.nid = n.nid WHERE c.field_vocabulary_nid = %d AND n.type = '%s'"), $variables['nid'], NEOLOGISM_CT_PROPERTY));
    $variables['author_list'] = array();
    foreach($variables['authors'] as $uid) {
      $author = user_load($uid);
      $name = empty($author->profile_neologism_fullname) ? $author->name : $author->profile_neologism_fullname;
      if ($author->profile_neologism_homepage) {
        $variables['author_list'][] = l($name, $author->profile_neologism_homepage);
      }
      else {
        $variables['author_list'][] = l($name, 'user/' . $uid);
      }
    }
  }
  else if ($variables['type'] == NEOLOGISM_CT_CLASS || $variables['type'] == NEOLOGISM_CT_PROPERTY) {
    $variables['is_class'] = $variables['type'] == NEOLOGISM_CT_CLASS;
    $variables['template_file'] = 'node-neo_term';
    $variables['term_uri'] = $variables['vocabulary']->namespace_uri . $variables['title'];
    $variables['qname'] = $variables['vocabulary']->prefix . ':' . $variables['title'];
    $variables['term_label'] = $variables['field_label'][0]['value'];
    $variables['term_comment'] = $variables['field_comment'][0]['value'];
    $variables['has_custom_namespace'] = !empty($variables['vocabulary']->field_custom_namespace[0]['value']);
    $variables['original_body'] = check_markup($variables['original_body'], $variables['format'], FALSE);
    $variables['superclasses'] = _neologism_themed_qnames($variables['field_superclass2']);
    $variables['disjoints'] = _neologism_themed_qnames($variables['field_disjointwith2']);
    $variables['domains'] = _neologism_themed_qnames($variables['field_domain2']);
    $variables['ranges'] = _neologism_themed_qnames($variables['field_range2']);
    $variables['inverses'] = _neologism_themed_qnames($variables['field_inverse2']);
    $variables['superproperties'] = _neologism_themed_qnames($variables['field_superproperty2']);
    
    // dummy fields
    $variables['subclasses'] = _neologism_themed_qnames($variables['subclasses']);
    
    $variables['types'] = array();
    if (!empty($variables['field_fp'][0]['value'])) {
      $variables['types'][] = trim(theme('neologism_qname', 'owl:FunctionalProperty'));
    }
    if (!empty($variables['field_ifp'][0]['value'])) {
      $variables['types'][] = trim(theme('neologism_qname', 'owl:InverseFunctionalProperty'));
    }
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

/**
 * Output the HTML version of the vocabulary specified as input.
 */
function neologism_export_html($node) {

  // @D7: Replace with drupal_add_html_head_link()
  drupal_add_link(array(
    'rel' => 'alternate',
    'type' => 'application/rdf+xml',
    'href' => url($node->path . '.rdf'),
    'title' => 'RDF Schema (in RDF/XML)',
  ));
  // @D7: Replace with drupal_add_html_head_link()
  drupal_add_link(array(
    'rel' => 'alternate',
    'type' => 'text/turtle',
    'href' => url($node->path . '.ttl'),
    'title' => 'RDF Schema (in Turtle)',
  ));

  $output = '<div class="vocabulary-view">';
  
  $image = theme('image', drupal_get_path('module', 'neologism') .'/images/icons/rdf_w3c_icon.48.gif', t('RDF/XML version of this vocabulary.'), t('RDF/XML version of this vocabulary.'));
  $link = l($image, $node->path.'.rdf', array('html' => TRUE, 'attributes' => Array('class' => 'icon')));
  $image = theme('image', drupal_get_path('module', 'neologism') .'/images/icons/n3.gif', t('Turtle version of this vocabulary.'), t('Turtle version of this vocabulary.'));
  $link2 = l($image, $node->path.'.ttl', array('html' => TRUE, 'attributes' => Array('class' => 'icon')));
  $output .= '<div id="icons">'.$link.'&nbsp;'.$link2.'</div>';
  
  list($classes, $superclasses_array, $disjoin_classes_array) = _neologism_get_all_classes($node, TRUE, TRUE);
  usort($classes, '_neologism_cmp_2array_alphabetically');
  
  list($properties, $inverse_properties_array) = _neologism_get_all_properties($node, TRUE);
  usort($properties, '_neologism_cmp_2array_alphabetically');
  
  $html = ''; $author_count = 0;
  foreach($node->authors as $uid) {
    $author_count++;
    $author = user_load(array('uid' => $uid));
    $html .= '<div id="author-value">';
    $name = empty($author->profile_neologism_fullname) ? $author->name : $author->profile_neologism_fullname;
    if ($author->profile_neologism_homepage) {
      $html .= l($name, $author->profile_neologism_homepage);
    } else {
      $html .= l($name, 'user/' . $uid);
    }
    if ($author->profile_neologism_affiliation) {
      if ($author->profile_neologism_affiliation_homepage) {
        $html .= ' (' . l($author->profile_neologism_affiliation, $author->profile_neologism_affiliation_homepage) . ')';
      } else {
        $html .= ' (' . $author->profile_neologism_affiliation . ')';
      }
    }
    if ($author->profile_neologism_show_email) {
      $html .= ' <i>(' . l('Email', 'mailto:' . $author->mail) . ')</i>';
    }
    $html .= '</div>';
  }
  if ($author_count) {
    $output .= '<div id="authors" ><h3>'.($author_count == 1 ? t('Author') : t('Authors')).':</h3>' . $html . '</div>';
  }

  $output .= '<div id="last-update" ><h3>Last update:</h3><div id="last-update-value">'.format_date($node->changed, 'custom', 'd F Y').'</div></div>';
  
  $output .= '<div id="namespace"><h3>Namespace URI:</h3> <div id="namespace-value">'.$node->namespace_uri.'</div></div>';

  // Abstract
  if ($node->abstract) {
    $output .= '<div id="abstract" class="neologism field"><div class="block title"><h3>Abstract</h3></div> ' . $node->abstract . ' </div>';
  }

  // Content type in urls are with - instead of _
  $vocabulary_url_str = str_replace('_', '-', NEOLOGISM_CT_VOCABULARY);
  $class_url_str = str_replace('_', '-', NEOLOGISM_CT_CLASS);
  $property_url_str = str_replace('_', '-', NEOLOGISM_CT_PROPERTY);
  
  //Vocabulary at glance
  if ($classes || $properties) {
    $output .= '<h2 id="sec_glance" class="neologism">All terms at a glance</h2>';
    $output .= '<div id="atglance">';
  }
  
  // TODO: loop through the classes and properties and populate the atglance and complete reference
  // the current fashion is not optimized as we need to read each class/prop twice
  // Display the Classes
  if ($classes) {
    $output .= '<p><strong>Classes:</strong> ';
    $count = 0; 
    foreach ($classes as $class) {
      if( $count > 0 ) 
        $output .= ' | ';
      $output .= '<a href="#' . $class['id'] . '" title="class ' . check_plain($node->title) . ':'. check_plain($class['id']) . '">' . check_plain($class['id']) . '</a>';
      $count++;
    }
    $output .= '</p>';
  }

  // Display the Properties
  if ($properties) {
    $output .= $classes ? '<br/>' : ''; 
    $output .= '<p><strong>Properties:</strong> ';
    $count = 0;
    foreach ($properties as $property) {
      if( $count > 0 ) { 
          $output .= ' | ';
      }
      $output .= '<a href="#' . $property['id'] . '" title="property ' . check_plain($node->title) . ':'. check_plain($property['id']) . '">' . check_plain($property['id']) . '</a>';
      $count++;
    }
    $output .= '</p>';
  }
  
  if ($classes || $properties) {
    $output .= '</div>';
  }
  
  //ext_load_library();
  drupal_add_js( array('neologism' => array('voc_id' => $node->nid, 'voc_title' => $node->prefix)), 'setting');
  
  $tree_view_class = 'treepanel-fullwidth';
  if ($classes && $properties) {
    $output .= '<div id="treeview-panel" class="clearfix">';
    $tree_view_class = 'treepanel-halfwidth';
  }
  
  if ($classes) {
    $output .= '<div id="class-tree" class="'.$tree_view_class.'"></div>';
    
    $opts = array('absolute' => TRUE);
    $json_url = url('neologism/json/classestree', $opts);
      
    //create the variable into the namespace neologism
    // eg: Drupal.settings.neologism.json_url
    drupal_add_js( array('neologism' => array('json_url' => $json_url)), 'setting');
    drupal_add_js( drupal_get_path('module', 'neologism') . '/js/class_treeview.js');
  }
  
  if ($properties) {
    // object property hierarchy tree
    $output .= '<div id="object-property-tree" class="'.$tree_view_class.'"></div>';
    
    $opts = array('absolute' => TRUE);
    $json_url = url('neologism/json/objectpropertytree', $opts);
    drupal_add_js( array('neologism' => array('property_json_url' => $json_url)), 'setting');
    drupal_add_js( drupal_get_path('module', 'neologism') . '/js/object_property_treeview.js');
  }
  
  if ($classes && $properties) {
    $output .= '</div>';  
  }
  
  $base = base_path();
  $swf = $base . drupal_get_path('module','neologism').'/includes/Diagram.swf';
  $img_legend = $base . drupal_get_path('module','neologism').'/includes/legend.png';
  $edit_mode = user_access('edit vocabularies');
  $vocabulary = $base.$node->path;
  $token = drupal_get_token();
  $width = '100%';
  $height = '600px';
  
  $flex_params = "editMode=$edit_mode&vocabulary=$vocabulary&token=$token";
  $output .= '
    <div id="diagram-panel">
    <h2>Overview diagram</h2>
    <object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"
        id="Diagram" width="'.$width.'" height="'.$height.'"
        codebase="http://fpdownload.macromedia.com/get/flashplayer/current/swflash.cab">
            <param name="movie" value="'.$swf.'" />
            <param name="quality" value="high" />
            <param name="bgcolor" value="#ffffff" />
            <param name="flashVars" value="'.$flex_params.'" />
            <param name="allowScriptAccess" value="sameDomain" />
            <embed src="'.$swf.'" quality="high" bgcolor="#ffffff"
                width="'.$width.'"
                height="'.$height.'"
                name="Diagram"
                align="middle"
                play="true"
                loop="false"
                quality="high"
                allowScriptAccess="sameDomain"
                type="application/x-shockwave-flash"
                pluginspage="http://www.adobe.com/go/getflashplayer"
                flashVars="'.$flex_params.'" />
    </object>
  </div>';
  
  // insert the vocabulary's dscription
  $output .= '<div id="description">'.$node->body.'</div>';


  //Display complete reference
  $output .= '<div class="reference">';
  if ($classes) {
    $output .= '<h2 id="sec_classes">Classes</h2>';
    foreach ($classes as $term) {
      $loaded_node = node_load($term['nid']);
      
      _neologism_normalize_implicit_values($disjoin_classes_array, $loaded_node->qname, $loaded_node->field_disjointwith2);
      
      // create a dummy variable for the neo_term.tpl
      _neologism_normalize_implicit_values($superclasses_array, $loaded_node->qname, $loaded_node->subclasses);
      //$loaded_node->subclasses = $superclasses_array;
      
      $output .= node_view($loaded_node, TRUE, FALSE, TRUE);
    }
  }
   
  if ($properties) {
    $output .= '<h2 id="sec_properties">Properties</h2>';
    foreach ($properties as $term) {
      $loaded_node = node_load($term['nid']);
      _neologism_normalize_implicit_values($inverse_properties_array, $loaded_node->qname, $loaded_node->field_inverse2);
      $output .= node_view($loaded_node, TRUE, FALSE, TRUE);
    }
  }
  $output .= '</div>';
 
  $output .= '</div>';
  
  return $output;
}

