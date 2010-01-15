<?php

define("SPARQL_QUERY_ERROR", "There is a NULL result in the SPARQL query <br/> 
      </br>Please check you ontology file, or create a new entry for the development team at ".
      l('Neologism at Google code','http://code.google.com/p/neologism/issues/list', array('html' => TRUE, 'attributes' => Array('title' => 'http://code.google.com/p/neologism/issues/list')))
      ." and do not forget to attach your ontology file to it.");

function neologism_import_form($form_state) {

	$form['vocabulary_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Vocabulary URI'),
    '#required' => TRUE,
    '#default_value' => $form_state['values']['vocabulary_uri'],
    '#description' => "Enter the URI of the vocabulary to import in Neologism. Make sure it finishes by either / or #.",
  );
  /*
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Prefix'),
    '#required' => TRUE,
    '#default_value' => $form_state['values']['prefix'],
    '#description' => "Choose a prefix for this vocabulary. This prefix will be used by default in Neologism.",
  );
  $form['ns_uri'] = array(
    '#type' => 'textfield',
    '#title' => t('Namespace URI'),
    '#required' => FALSE,
    '#default_value' => isset($form_state['values']['ns_uri']) ? $form_state['values']['ns_uri'] : NULL,
    '#description' => "Do not use unless the vocabulary defines a different namespace than the one above. Make sure it finishes by either / or #.",
  );
  $form['vocabulary_id'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary to import in'),
    '#required' => TRUE,
    '#default_value' => $form_state['values']['vocabulary_id'],
    '#options' => array('<create>', 1),
    '#description' => 'Select the existing vocabulary to import the new vocabulary in. If you want to create a new vocabulary, select &lt;create&gt;.',
  );
  */
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  
  return $form;
}

function neologism_import_form_submit($form, &$form_state) {
  $form_state['storage']['values'] = $form_state['values'];
  $form_state['rebuild'] = true;

  //$fetched_terms = evoc_fetch_vocabulary(
  //$form_state['values']['vocabulary_uri'], $form_state['values']['prefix'], TRUE, $form_state['values']['ns_uri']);
  
  _neologism_import_vocabulary($form_state['values']['vocabulary_uri']);
}

/**
 * 
 * @param object $form_state
 * @return 
 */
function neologism_import_fromfile_form($form_state) {
  $form['#attributes']['enctype'] = 'multipart/form-data';
  //$form['#attributes'] = array('enctype' => "multipart/form-data");
  //'upload' will be used in file_check_upload()
  $form['file_upload'] = array(
    '#type' => 'file',
    '#title' => t('Select vocabulary file to import into Neologism'),
    '#required' => FALSE,
    '#description' => t('Click "Browse" to select a file to upload. 
      The file should has .n3 or .rdf extension.  
      The maximum file size per upload is %size MB.', array('%size' => round(file_upload_max_size()/(1024*1024), 0))),
    '#weight' => -5,
  );
    
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Ontology prefix'),
    '#required' => FALSE,
    //'#default_value' => $form_state['values']['prefix'],
    '#description' => "Choose a ontology prefix to force Neologism to create a vocabulary with such a prefix.",
  );
    
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );  
  
  return $form;    
}

function neologism_import_fromfile_form_validate($form, &$form_state) {
  if (isset($form['file_upload'])) {
    $validators = array(
      'file_validate_extensions' => array('rdf', 'n3')
    );
 
    $file = file_save_upload('file_upload', $validators);
    if (!$file) {
      form_set_error('upload', 'You must select a valid file to upload.');
    }
    else {
      if( isset($form_state['values']['prefix']) ) {
        $form_state['values']['title'] = $form_state['values']['prefix'];
      }
      else {
        //$form_state['values']['title'] = $file->filename;
        $form_state['values']['title'] = NULL;
      }
      $form_state['values']['uploaded_file'] = $file;
    }
  }
}

/**
 * This function check if the current $uri belong to the $vocabulary
 * @param string $vocabulary 
 * @param Resource $uri
 * @return TRUE if $uri == $vocabulary
 */
function _neologism_is_from_vocabulary_imported( $vocabulary, $uri) {
  // TODO fix the error in that function at rdf.api.inc
  if( rdf_is_valid_uri($uri) ) {
    $start = strrpos($uri, '/') + 1; 
    $end = strrpos($uri, '#'); 
    if( $start && $end && ($voc = substr($uri, $start, $end - $start)) ) {
      if( $voc == $vocabulary ) {
        return $voc.':'.substr($uri, $end + 1);
      }
    }
  }
  
  return NULL;
}

function neologism_import_fromfile_form_submit($form, &$form_state) {
  $uploaded_file = $form_state['values']['uploaded_file'];
  $vocabulary_name = trim($form_state['values']['title']);
  
  _neologism_import_vocabulary($uploaded_file->filepath, $vocabulary_name);
}

/**
 * 
 * @param object $voc_term
 * @return 
 */
function neologism_new_node_vocabulary( array $voc_term ) {
  module_load_include('inc', 'node', 'node.pages');
  
  global $language;  
  $node = new StdClass();
  $node->type = NEOLOGISM_CT_VOCABULARY;
  node_object_prepare($node);
  
  $node->title = $voc_term['id'];
  $node->field_title[0]['value'] = isset($voc_term['title']) ? $voc_term['title'] : $node->title;
  $node->field_authors[0]['uid'] = $node->uid;
  $node->teaser = check_plain($voc_term['description']);
  $node->body = $node->teaser;
  
  // all our validations has been added for form processing so we need to check our values here
  // in a different way
  if( !_neologism_is_valid_id($node->title) ) {
    drupal_set_message( t($node->title.'. '.MSG_INVALID_ID), 'error');
    return NULL;
  }
  
  // check if there is some voc with the same id
  $count = db_result(db_query(db_rewrite_sql("select count(*) from {node} where title = '%s' and type = '%s'"), $node->title, NEOLOGISM_CT_VOCABULARY));
  if( $count > 0 ) {
    drupal_set_message( t($node->title.' '.MSG_ID_ALREADY_IN_USE), 'error');
    return NULL;
  }

  $node->language = $language->language;
  $node->promote = 1;
  
  node_save($node);
  if ($node->nid) {
    watchdog('content', '@type: added %title using neologism API.', array('@type' => $node->type, '%title' => $node->title), WATCHDOG_NOTICE, l(t('view'), "node/$node->nid"));
    return $node->nid;
  }

  drupal_set_message( t($node->title.', can not be created'), 'error');
  return NULL;
}

function neologism_new_node_class($vocabulary_nid, $term) {
  module_load_include('inc', 'node', 'node.pages');
  
  global $language;  
  $node = new StdClass();
  $node->type = NEOLOGISM_CT_CLASS;
  node_object_prepare($node);
  
  $node->title = $term['id'];
  if( !_neologism_is_valid_id($node->title) ) {
    drupal_set_message( t($node->title.'. '.MSG_INVALID_ID), 'error');
    return NULL;
  }
  
  $count = db_result(db_query(db_rewrite_sql("select count(*) from {content_field_vocabulary} c inner join {node} n on c.nid = n.nid 
    where c.field_vocabulary_nid = %d and title = '%s' and type = '%s'"), $vocabulary_nid, $node->title, NEOLOGISM_CT_CLASS));
  if( $count > 0 ) {
    drupal_set_message( t($node->title.', '.MSG_ID_ALREADY_IN_USE), 'error');
    return NULL;
  }
  
  $node->field_vocabulary[0]['nid'] = $vocabulary_nid;
  $node->field_label[0]['value'] = $term['label'];
  $node->field_comment[0]['value'] = $term['comment'];
  
  if( count($term[SUPERCLASSES]) > 0 ) {
    foreach( $term[SUPERCLASSES] as $value ) {
      if( isset($value) ) {
        $node->field_superclass2[] = array('evoc_term' => $value);
      }
    }
  }
  else {
    $node->field_superclass2[0] = array('evoc_term' => NULL);
  }
  
  if( count($term[DISJOINTS_WITH]) > 0 ) {
    foreach( $term[DISJOINTS_WITH] as $value ) {
      if( isset($value) ) {
        $node->field_disjointwith2[] = array('evoc_term' => $value);
      }
    }
  }
  else {
    $node->field_disjointwith2[0] = array('evoc_term' => NULL);
  }
  
  $node->field_comment[0]['value'] = $term['comment'];
  
  $node->language = $language->language;
  node_save($node);
  if( $node->nid ) {
    watchdog('content', '@type: added %title using neologism API.', array('@type' => $node->type, '%title' => $node->title), WATCHDOG_NOTICE, l(t('view'), "node/$node->nid"));
    return $node->nid;
  }

  return NULL;
}

function neologism_new_node_property($vocabulary_nid, $term) {
  module_load_include('inc', 'node', 'node.pages');
  
  global $user, $language;  
  $node = new StdClass();
  $node->type = NEOLOGISM_CT_PROPERTY;
  node_object_prepare($node);
  
  $node->title = $term['id'];
  if( !_neologism_is_valid_id($node->title) ) {
    drupal_set_message( t($node->title.'. '.MSG_INVALID_ID), 'error');
    return NULL;
  }
  
  $count = db_result(db_query(db_rewrite_sql("select count(*) from {content_field_vocabulary} c inner join {node} n on c.nid = n.nid 
    where c.field_vocabulary_nid = %d and title = '%s' and type = '%s'"), $vocabulary_nid, $node->title, NEOLOGISM_CT_PROPERTY));

  if( $count > 0 ) {
    drupal_set_message( t($node->title.', '.MSG_ID_ALREADY_IN_USE), 'error');
    return NULL;
  }
  
  $node->field_vocabulary[0]['nid'] = $vocabulary_nid;
  $node->field_label[0]['value'] = $term['label'];
  $node->field_comment[0]['value'] = $term['comment'];
  
  if( count($term['domain']) > 0 ) {
    foreach( $term['domain'] as $value ) {
      if( isset($value) ) {
        $node->field_domain2[] = array('evoc_term' => $value);
      }
    }
  } else {
    $node->field_domain2[0] = array('evoc_term' => NULL);
  }
  
  if( count($term['range']) > 0 ) {
    foreach( $term['range'] as $value ) {
      if( isset($value) ) {
        $node->field_range2[] = array('evoc_term' => $value);
      }
    }
  } else {
    $node->field_range2[0] = array('evoc_term' => NULL);
  }
  
  if( count($term['subpropertyof']) > 0 ) {
    foreach( $term['subpropertyof'] as $value ) {
      if( isset($value) ) {
        $node->field_superproperty2[] = array('evoc_term' => $value);
      }
    }
  } else {
    $node->field_superproperty2[0] = array('evoc_term' => NULL);
  }
  
  $node->field_fp[0]['value'] = NULL;
  $node->field_ifp[0]['value'] = NULL;
  foreach( $term['owl_type'] as $value ) {
    if( isset($value) ) {
      if( $value == 'owl:FunctionalProperty' ) {
        $node->field_fp[0]['value'] = TRUE;      
      }
      else if( $value == 'owl:InverseFunctionalProperty' ) {
        $node->field_ifp[0]['value'] = TRUE;
      }
    }
  }
    
  //$node->field_inverse2[0]['value'] = $term['inverse'];
  /*
  // inverse
  foreach( $term['inverse'] as $value ) {
    if( isset($value) ) {
      $node['field_inverse2'][] = array('evoc_term' => $value);
    }
  }
  */
    
  $node->language = $language->language;
  node_save($node);
  if( $node->nid ) {
    watchdog('content', '@type: added %title using neologism API.', array('@type' => $node->type, '%title' => $node->title), WATCHDOG_NOTICE, l(t('view'), "node/$node->nid"));
    return $node->nid;
  }

  return NULL;
}

function _neologism_import_vocabulary($resource, $title = NULL) {
  include(RDFAPI_INCLUDE_DIR . "RdfAPI.php");
  include(RDFAPI_INCLUDE_DIR . "syntax/RdfSerializer.php");
    
  $model = ModelFactory::getMemModel();
  $model->load($resource);
  
  // there is a convention. The filename is the vocabulary's name if the document fail in query for document's name
  $vocabulary_name = trim($title);
  //if( $vocabulary_name != NULL ) {
  //  $vocabulary_name = substr($vocabulary_name, 0, strrpos($vocabulary_name, '.'));
  //}
  $vocabulary_title = "";
  
  $prefixes = '
    prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
    prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    prefix owl: <http://www.w3.org/2002/07/owl#>
    prefix foaf: <http://xmlns.com/foaf/0.1/>';
  
  /*
  $query = $prefixes . '
    prefix dc: <http://purl.org/dc/terms/>
    
    select ?s ?p ?o
    where { ?s ?p ?o } 
  ';
  $query_result2 = $model->sparqlQuery($query);
  */
 
  $prefixes = '
    prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
    prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
    prefix owl: <http://www.w3.org/2002/07/owl#>
    prefix foaf: <http://xmlns.com/foaf/0.1/>';
  
  $query = $prefixes . '
    prefix dc: <http://purl.org/dc/terms/>
    
    select ?title ?document
    where { ?document rdf:type foaf:Document; rdfs:label ?title } 
  ';
  
  $query_result = $model->sparqlQuery($query);
  // patch to the old DC URI, could be an old Neologism exported ontology
  if( $query_result == NULL ) {
    $query = $query = $prefixes . '
      prefix dc: <http://purl.org/dc/elements/1.1/>
      
      select ?title ?document
      where { ?document rdf:type foaf:Document; rdfs:label ?title } 
    ';  
    
    $query_result = $model->sparqlQuery($query);
  }
  
  if( $query_result == NULL || !isset($query_result[0]['?title'])  ) {
    $query = $prefixes . '
      prefix dc: <http://purl.org/dc/terms/>
      
      select * 
      where { 
        {?document rdf:type owl:Ontology; dc:title ?title}.
          OPTIONAL {?document dc:description ?description }.
          OPTIONAL {?document dc:title ?title}
      }
    ';
    
    $query_result = $model->sparqlQuery($query);
    // if $query_result == NULL query using the old DC URI
    if( $query_result == NULL ) {
      $query = $prefixes . '
        prefix dc: <http://purl.org/dc/elements/1.1/>
        
        select * 
        where { 
          {?document rdf:type owl:Ontology; dc:title ?title}.
            OPTIONAL {?document dc:description ?description }.
            OPTIONAL {?document dc:title ?title} 
        }
      ';
      $query_result = $model->sparqlQuery($query);
    }
    
  }
  
  if( is_array($query_result) ) {
    foreach ($query_result as $result) {
      if (isset($result['?title']->label) ) {
        $vocabulary_title = $result['?title']->label;
        break;
      }
    }
    
    foreach ($query_result as $result) {
      if (isset($result['?description']->label) ) {
        $vocabulary_description = $result['?description']->label;
        break;
      }
    }
  }
  
  
  //TODO: write a message for the user when the ontology title is not provided
  if( $vocabulary_title == "" ) {
    drupal_set_message('You document does not appear to contain a title. Your document does not appear to contain a title. Neologism has selected as title the imported filename.', 'warning');
    $vocabulary_title = $vocabulary_name;
  }
  
  if( $vocabulary_name == NULL && isset($query_result[0]['?document']->uri) ) {
    $query_result[0]['?document']->uri = trim($query_result[0]['?document']->uri);
    $vocabulary_name = substr($query_result[0]['?document']->uri, strrpos($query_result[0]['?document']->uri, '/') + 1);
    if ( ($pos = strpos($vocabulary_name, '#')) ) {
      $vocabulary_name =  substr($vocabulary_name, 0, $pos);
    }
    
    // patch for neologism 1.x
    if( $vocabulary_name == 'rdf') {
      $vocabulary_name = substr($query_result[0]['?document']->uri, 0, strrpos($query_result[0]['?document']->uri, '/'));  
      $vocabulary_name = substr($vocabulary_name, strrpos($vocabulary_name, '/') + 1);
    }  
  }
  
  // SPARQL queries are stored in an array.
  $term_queries = array();
  // Typical SPARQL queries to retrieve properties/classes, working for all major vocabularies...
  
  $term_queries[] = array(
    'type' => 'class',
    'query' => '
      prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
      prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
      prefix owl: <http://www.w3.org/2002/07/owl#>
      select *
      where { {?class rdf:type owl:Class} union {?class rdf:type rdfs:Class}.
        OPTIONAL {?class rdfs:label ?label}.
        OPTIONAL {?class rdfs:subClassOf ?superclass}.
        OPTIONAL {?class owl:disjointWith ?disjointwith}.
        OPTIONAL {?class rdfs:comment ?comment}.
      }
      '
  );
  
  $term_queries[] = array(
    'type' => 'property',
    'query' => ' 
      prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
      prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#>
      prefix owl: <http://www.w3.org/2002/07/owl#>
      select *
      where {
        {?property rdf:type owl:DatatypeProperty} 
      union 
        {?property rdf:type owl:ObjectProperty}.
          OPTIONAL {?property rdf:type ?t . FILTER (?t = owl:FunctionalProperty || ?t = owl:InverseFunctionalProperty ) }.
      union 
        {?property rdf:type rdf:Property}.
          OPTIONAL {?property rdfs:label ?label}.
          OPTIONAL {?property rdfs:domain ?domain}.
          OPTIONAL {?property rdfs:range ?range}.
          OPTIONAL {?property rdfs:subPropertyOf ?subpropertyof}.
          OPTIONAL {?property rdfs:comment ?comment}.
          OPTIONAL {?property rdf:type ?t2 }.
      }
      limit 1000'
  );

  // Loop through all the various queries in order to extract as many classes and properties as possible.
  $fetched_terms = array();
  foreach ($term_queries as $query) {
    $query_result = $model->sparqlQuery($query['query']);
    
    if( !$query_result ) {
      drupal_set_message(SPARQL_QUERY_ERROR, 'error');
      return;
    }
    
    // Parse the SPARQL results and extract the relevant terms.
    foreach( $query_result as $res_term ) {
      
      if( isset($res_term['?class']) ) {
        $type = '?class';
      }
      else if( isset($res_term['?property']) ) {
        $type = '?property';
      };

      $res_term[$type]->uri = trim($res_term[$type]->uri);
      $term_qname = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term[$type]->uri); 
      if( !$term_qname ) {
        $term_qname = neologism_rdf_uri_to_qname($res_term[$type]->uri); //rdf_uri_to_qname($res_term[$type]->uri);  
      }
      $term_qname_parts = explode(':', $term_qname);
      $term_prefix = $term_qname_parts[0];

      $term = array();
      $term['id'] = $term_qname_parts[1];
      $term['type'] = $type;
      $term['label'] = isset($res_term['?label']->label) ? $res_term['?label']->label : NULL;
      $term['comment'] = isset($res_term['?comment']->label) ? trim($res_term['?comment']->label) : NULL;
      
      switch( $type ) {
        case '?class':
          $term[SUPERCLASSES] = array();
          if ( isset($res_term['?superclass']->uri) ) {
            $res_term['?superclass']->uri = trim($res_term['?superclass']->uri);
            $term[SUPERCLASSES][] = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term['?superclass']->uri); 
            if( !$term[SUPERCLASSES][0] ) {
              $term[SUPERCLASSES][0] = neologism_rdf_uri_to_qname($res_term['?superclass']->uri);//rdf_uri_to_qname($res_term['?superclass']->uri);  
            }
          }
          
          $term[DISJOINTS_WITH] = array();
          if ( isset($res_term['?disjointwith']->uri) ) {
            $res_term['?disjointwith']->uri = trim($res_term['?disjointwith']->uri);
            $term[DISJOINTS_WITH][] = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term['?disjointwith']->uri); 
            if( !$term[DISJOINTS_WITH][0] ) {
              $term[DISJOINTS_WITH][0] = neologism_rdf_uri_to_qname($res_term['?disjointwith']->uri);  
            }
          }
          
          break;

        case '?property':
          $term['subpropertyof'] = array();
          if( isset($res_term['?subpropertyof']->uri) ) {
            $res_term['?subpropertyof']->uri = trim($res_term['?subpropertyof']->uri);
            $term['subpropertyof'][] = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term['?subpropertyof']->uri); 
            if( !$term['subpropertyof'][0] ) {
              $term['subpropertyof'][0] = neologism_rdf_uri_to_qname($res_term['?subpropertyof']->uri);  
            }
          }
          
          $term['domain'] = array();
          if( isset($res_term['?domain']->uri) ) {
            $res_term['?domain']->uri = trim($res_term['?domain']->uri);
            $term['domain'][] = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term['?domain']->uri); 
            if( !$term['domain'][0] ) {
              $term['domain'][0] = neologism_rdf_uri_to_qname($res_term['?domain']->uri);  
            }
          }
          
          $term['range'] = array();
          if( isset($res_term['?range']->uri) ) {
            $res_term['?range']->uri = trim($res_term['?range']->uri);
            $term['range'][] = _neologism_is_from_vocabulary_imported($vocabulary_name, $res_term['?range']->uri); 
            if( !$term['range'][0] ) {
              $term['range'][0] = neologism_rdf_uri_to_qname($res_term['?range']->uri);  
            }
          }
          
          $term['owl_type'] = array();
          if( isset($res_term['?t']->uri) ) {
            $res_term['?t']->uri = trim($res_term['?t']->uri);
            $term['owl_type'][] = neologism_rdf_uri_to_qname($res_term['?t']->uri); 
          }
          // FIXME
          // temporal patch we need to figure out how to get in a better way the properties type
          else if( isset($res_term['?t2']->uri) ) {
            $res_term['?t2']->uri = trim($res_term['?t2']->uri);
            $owl_type = neologism_rdf_uri_to_qname($res_term['?t2']->uri);
            if( $owl_type == 'owl:InverseFunctionalProperty' || $owl_type == 'owl:FunctionalProperty') {
              $term['owl_type'][] = $owl_type;
            }
          }
          
          break;
      }

      // filter process
      $term_exist = FALSE;
      $just_updated = FALSE;
      foreach( $fetched_terms as &$term_value ) {
        if( $term_value['id'] == $term['id'] && $term_value['type'] == $term['type'] ) {
          $term_exist = TRUE;
          if( $term['type'] == '?class' ) {
            if( !isset($term_value['label']) && isset($term['label']) ) {
              $term_value['label'] = $term['label'];
            }
            if( !isset($term_value['comment']) && isset($term['comment']) ) {
              $term_value['comment'] = $term['comment'];
            }
            if ( isset($term[SUPERCLASSES][0]) && !in_array($term[SUPERCLASSES][0], $term_value[SUPERCLASSES]) ) {
              $term_value[SUPERCLASSES][] = $term[SUPERCLASSES][0];
            }
            if ( isset($term[DISJOINTS_WITH][0]) && !in_array($term[DISJOINTS_WITH][0], $term_value[DISJOINTS_WITH]) ) {
              $term_value[DISJOINTS_WITH][] = $term[DISJOINTS_WITH][0];
            }
          }
          elseif ( $term['type'] == '?property') {
            if( !isset($term_value['label']) && isset($term['label']) ) {
              $term_value['label'] = $term['label'];
            }
            if( !isset($term_value['comment']) && isset($term['comment']) ) {
              $term_value['comment'] = $term['comment'];
            }
            if ( isset($term['subpropertyof'][0]) && !in_array($term['subpropertyof'][0], $term_value['subpropertyof']) ) {
              $term_value['subpropertyof'][] = $term['subpropertyof'][0];
            }
            if ( isset($term['domain'][0]) && !in_array($term['domain'][0], $term_value['domain']) ) {
              $term_value['domain'][] = $term['domain'][0];
            }
            if ( isset($term['range'][0]) && !in_array($term['range'][0], $term_value['range']) ) {
              $term_value['range'][] = $term['range'][0];
            }
            if ( isset($term['owl_type'][0]) && !in_array($term['owl_type'][0], $term_value['owl_type']) ) {
              $term_value['owl_type'][] = $term['owl_type'][0];
            }
          }
          
          // jump if the term exists
          break;  
        }
      }
      unset($term_value);
        
      // if not just_updated and not exists such a term add it to the array
      if ( !$term_exist ) {
        $fetched_terms[] = $term;
      }
    }
  } // foreach
  
  $voc_nid = neologism_new_node_vocabulary( array('id' => $vocabulary_name, 'title' => $vocabulary_title, 'description' => $vocabulary_description) );
  if( $voc_nid ) {
    foreach ($fetched_terms as $term) {
      if ($term['type'] == '?class') {
        neologism_new_node_class($voc_nid, $term);
      }
      elseif ($term['type'] == '?property') {
        neologism_new_node_property($voc_nid, $term);
      }
    }   
  }  
}
