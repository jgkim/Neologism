<?php
// $Id: devel_generate.inc,v 1.1.2.12 2007/12/15 01:56:40 dww Exp $
// If not in 'safe mode', increase the maximum execution time:
if (!ini_get('safe_mode')) {
  set_time_limit(240);
}

/**
 * Generate some random users.
 *
 * @param $num
 *  Number of users to generate.
 * @param $kill
 *  Boolean that indicates if existing users should be removed first.
 */
function devel_create_users($num, $kill) {
  $url = parse_url($GLOBALS['base_url']);
  if ($kill) {
    db_query('DELETE FROM {users} WHERE uid > 1');
    drupal_set_message(t('Users deleted.'));
  }
  for ($i = 0; $i < $num; $i++) {
    $uid = db_next_id('{users}_uid');
    $length = rand(4, 12);
    $name = devel_generate_word($length);
    $pass = md5(user_password());
    $mail = $name .'@'. $url['host'];
    $now = time();
    db_query("INSERT INTO {users} (uid, name, pass, mail, status, created, access) VALUES (%d, '%s', '%s', '%s', %d, %d, %d)", $uid, $name, $pass, $mail, 1, $now, $now);
  }
  drupal_set_message(t('!num_users created.', array('!num_users' => format_plural($num, '1 user', '@count users'))));
}

function devel_generate_content($num_nodes, $num_comments, $title_length, $kill, $node_types = array()) {
  if ($kill) {
    $sql = 'SELECT nid FROM {node} WHERE type IN ('. implode(', ', array_fill(0, count($node_types), "'%s'")). ')';
    $result = db_query($sql, $node_types);
    while ($row = db_fetch_object($result)) {
      node_delete($row->nid);
    }
  }

  // Get user id.
  $users = devel_get_users();

  // Create $num_nodes pseudo-random nodes.
  devel_create_nodes($num_nodes, $users, $title_length, $node_types);
  drupal_set_message(t('%num nodes created.', array('%num' => $num_nodes)));

  $nodes = devel_get_nodes($next_nid);

  $comments = devel_get_comments();

  devel_create_comments($num_comments, $users, $nodes, $comments);
  drupal_set_message(t('%num comments created.', array('%num' => $num_comments)));
}

function devel_create_nodes($records, $users, $title_length = 8, $types = array()) {

  if (is_array($types)) {
    // Insert new data:
    for ($i = 1; $i <= $records; $i++) {
      $node->uid = $users[array_rand($users)];
      $node->type = $types[array_rand($types)];
      // Get the next nid without incrementing it.
      switch ($GLOBALS['db_type']) {
        case 'mysql':
        case 'mysqli':
          $next_nid = db_result(db_query("SELECT id FROM {sequences} WHERE name = '{node}_nid'")) + 1;
          break;
        case 'pgsql':
          $next_nid = db_result(db_query("SELECT currval('{node}_nid_seq')")) + 1;
          break;
      }


      $title = devel_create_greeking(rand(1, $title_length), TRUE);
      $node->title = $title;
      $node->body = "node #$next_nid ($node->type) - ". devel_create_content();
      $node->teaser = node_teaser($node->body);
      $node->filter = variable_get('filter_default_format', 1);
      $node->status = 1;
      $node->revision = rand(0,1);
      $node->promote = rand(0, 1);
      $node->comment = 2;
      $node->created = time();
      $node->changed = time();
      
      // TODO: add ability to disable terms
      if (1 || $add_terms) {
        devel_generate_add_terms($node);
      }

      // Save the node:
      node_save($node);

      // Setup a path:
      db_query("INSERT INTO {url_alias} (src, dst) VALUES ('%s', '%s')", "node/$node->nid", "node-$node->nid-$node->type");
      unset($node);
    }
  }
}


function devel_generate_add_terms(&$node) {
  $vocabs = taxonomy_get_vocabularies($node->type);
  // dsm($vocabs);
  foreach ($vocabs as $vocab) {
    $sql = "SELECT tid FROM {term_data} WHERE vid = %d ORDER BY RAND()";
    $result = db_query_range($sql, $vocab->vid, 0, 5);
    while ($row = db_fetch_object($result)) {
      if ($vocab->tags) {
        $node->taxonomy['tags'][] = $row->tid;
      }
      else {
        $node->taxonomy[$row->tid] = $row->tid;
      }
      if (!$vocab->multiple) {
        break;
      }
    }
  }
}


function devel_create_comments($records, $users, $nodes, $comments) {
  $users = array_merge($users, array('0'));
  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
    $comment->cid = db_next_id("{comments}_cid");
    $comment->nid = array_rand($nodes);

    switch ($i % 3) {
      case 1:
        $comment->pid = db_result(db_query("SELECT cid FROM {comments} WHERE pid = 0 AND nid = %d ORDER BY RAND() LIMIT 1", $comment->nid));
        break;
      case 2:
        $comment->pid = db_result(db_query("SELECT cid FROM {comments} WHERE pid > 0 AND nid = %d ORDER BY RAND() LIMIT 1", $comment->nid));
        break;
      default:
        $comment->pid = 0;
    }

    $comment->subject = "comment #". $comment->cid;
    $comment->comment = "body of comment #". $comment->cid;
    $comment->uid = $users[array_rand($users)];

    db_query("INSERT INTO {comments} (cid, nid, pid, uid, subject, comment, status, thread, timestamp) VALUES (%d, %d, %d, %d, '%s', '%s', %d, %d, %d)", $comment->cid, $comment->nid, $comment->pid, $comment->uid, $comment->subject, $comment->comment, 0, 0, time());
  }
  
  // update comments statistics
  foreach ($nodes as $nid => $type) {
    _comment_update_node_statistics($nid);
  }
}

function devel_generate_vocabs($records, $maxlength = 12, $types = array('story', 'blog', 'forum', 'page')) {
  $vocs = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
    $voc = array();
    $voc['name'] = devel_generate_word(rand(2, $maxlength));
    $voc['description'] = "description of ". $voc['name'];
    $voc['nodes'] = array_flip(array($types[array_rand($types)]));
    foreach ($voc['nodes'] as $key => $value) {
      $voc['nodes'][$key] = $key;
    }
    $voc['multiple'] = 1;
    $voc['required'] = 0;
    $voc['relations'] = 1;
    $voc['hierarchy'] = 1;
    $voc['weight'] = rand(0,10);

    taxonomy_save_vocabulary($voc);
    $vocs[] = $voc['name'];
  }
  return $vocs;
}

function devel_generate_terms($records, $vocs, $maxlength = 12) {
  $terms = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {

    switch ($i % 2) {
      case 1:
        $term['vid'] = $vocs[array_rand($vocs)];
        // dont set a parent. handled by taxonomy_save_term()
        // $term->parent = 0;
        break;
      case 2:
      default:
        $parent = db_fetch_object(db_query_range("SELECT t.tid, v.vid FROM {term_data} t INNER JOIN {vocabulary} v ON t.vid = v.vid ORDER BY RAND()", 0, 1));
        $term['parent'] = array($parent->tid);
        $term['vid'] = $parent->vid;
        break;
    }

    $term['name'] = devel_generate_word(rand(2, $maxlength));
    $term['description'] = "description of ". $term['name'];
    $term['weight'] = rand(0,10);

    $status = taxonomy_save_term($term);

    if ($status) {
      $terms[] = $term['name'];
    }

    unset($term);
  }
  return $terms;
}

function devel_generate_get_vocabs() {
  $vocs = array();
  $result = db_query("SELECT vid FROM {vocabulary}");
  while($voc = db_fetch_object($result)){
    $vocs[] = $voc->vid;
  }
  return $vocs;
}

function devel_generate_taxonomy_data($num_vocab, $num_terms, $title_length, $kill) {

  if ($kill) {
    db_query("DELETE FROM {term_data}");
    db_query("DELETE FROM {term_node}");
    db_query("DELETE FROM {term_hierarchy}");
    db_query("DELETE FROM {term_relation}");
    db_query("DELETE FROM {term_synonym}");
    db_query("DELETE FROM {vocabulary}");
    db_query("DELETE FROM {vocabulary_node_types}");
    switch ($GLOBALS['db_type']) {
      case 'mysql':
      case 'mysqli':
        db_query("UPDATE {sequences} SET id = '0' WHERE name = '{vocabulary}_vid'");
        db_query("UPDATE {sequences} SET id = '0' WHERE name = '{term_data}_tid'");
        db_query("ALTER TABLE {vocabulary} AUTO_INCREMENT = 1");
        db_query("ALTER TABLE {term_data} AUTO_INCREMENT = 1");
        break;
      case 'pgsql':
        db_query("SELECT setval('{vocabulary}_vid_seq', 1, false)");
        db_query("SELECT setval('{term_data}_tid_seq', 1, false)");
        break;
    }
    drupal_set_message(t('Deleted taxonomy.'));
  }

  $new_vocs = devel_generate_vocabs($num_vocab, $title_length);
  if (!empty($new_vocs)) {
    drupal_set_message(t('Created the following new vocabularies: !vocs', array('!vocs' => theme('item_list', $new_vocs))));
  }
  $vocs = devel_generate_get_vocabs();
  $new_terms = devel_generate_terms($num_terms, $vocs, $title_length);
  if (!empty($new_terms)) {
    drupal_set_message(t('Created the following new terms: !terms', array('!terms' => theme('item_list', $new_terms))));
  }
}

function devel_generate_word($length){
    srand((double)microtime()*1000000);

    $vowels = array("a", "e", "i", "o", "u");
    $cons = array("b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "u", "v", "w", "tr",
    "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl", "sh");

    $num_vowels = count($vowels);
    $num_cons = count($cons);

    while(strlen($word) < $length){
      $word .= $cons[rand(0, $num_cons - 1)] . $vowels[rand(0, $num_vowels - 1)];
    }

    return substr($word, 0, $length);
}

function devel_create_content() {
  $nparas = rand(1,12);
  $type = rand(0,3);

  $output = "";
  switch($type % 3) {
    case 1: // html
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60),1);
      }
      break;

    case 2: // brs only
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60),2);
      }
      break;

    default: // plain text
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60)) ."\n\n";
      }
  }

  return $output;
}

function devel_create_para($words, $type = 0) {
  $output = "";
  switch ($type) {
    case 1:
      $output .= "<p>";
      $output .= devel_create_greeking($words);
      $output = trim($output) ."</p>";
      break;

    case 2:
      $output .= devel_create_greeking($words);
      $output = trim($output) ."<br />";
      break;

    default:
      $output .= devel_create_greeking($words);
      $output = trim($output);
  }
  return $output;
}

function devel_create_greeking($words, $title = FALSE) {
  $dictionary = array("abbas", "abdo", "abico", "abigo", "abluo", "accumsan",
    "acsi", "ad", "adipiscing", "aliquam", "aliquip", "amet", "antehabeo",
    "appellatio", "aptent", "at", "augue", "autem", "bene", "blandit",
    "brevitas", "caecus", "camur", "capto", "causa", "cogo", "comis",
    "commodo", "commoveo", "consectetuer", "consequat", "conventio", "cui",
    "damnum", "decet", "defui", "diam", "dignissim", "distineo", "dolor",
    "dolore", "dolus", "duis", "ea", "eligo", "elit", "enim", "erat",
    "eros", "esca", "esse", "et", "eu", "euismod", "eum", "ex", "exerci",
    "exputo", "facilisi", "facilisis", "fere", "feugiat", "gemino",
    "genitus", "gilvus", "gravis", "haero", "hendrerit", "hos", "huic",
    "humo", "iaceo", "ibidem", "ideo", "ille", "illum", "immitto",
    "importunus", "imputo", "in", "incassum", "inhibeo", "interdico",
    "iriure", "iusto", "iustum", "jugis", "jumentum", "jus", "laoreet",
    "lenis", "letalis", "lobortis", "loquor", "lucidus", "luctus", "ludus",
    "luptatum", "macto", "magna", "mauris", "melior", "metuo", "meus",
    "minim", "modo", "molior", "mos", "natu", "neo", "neque", "nibh",
    "nimis", "nisl", "nobis", "nostrud", "nulla", "nunc", "nutus", "obruo",
    "occuro", "odio", "olim", "oppeto", "os", "pagus", "pala", "paratus",
    "patria", "paulatim", "pecus", "persto", "pertineo", "plaga", "pneum",
    "populus", "praemitto", "praesent", "premo", "probo", "proprius",
    "quadrum", "quae", "qui", "quia", "quibus", "quidem", "quidne", "quis",
    "ratis", "refero", "refoveo", "roto", "rusticus", "saepius",
    "sagaciter", "saluto", "scisco", "secundum", "sed", "si", "similis",
    "singularis", "sino", "sit", "sudo", "suscipere", "suscipit", "tamen",
    "tation", "te", "tego", "tincidunt", "torqueo", "tum", "turpis",
    "typicus", "ulciscor", "ullamcorper", "usitas", "ut", "utinam",
    "utrum", "uxor", "valde", "valetudo", "validus", "vel", "velit",
    "veniam", "venio", "vereor", "vero", "verto", "vicis", "vindico",
    "virtus", "voco", "volutpat", "vulpes", "vulputate", "wisi", "ymo",
    "zelus");

  $greeking = "";

  if (!$title) {
	  while ($words > 0) {
	    $sentence_length = rand(3,10);

	    $greeking .= ucfirst($dictionary[array_rand($dictionary)]);
	    for ($i = 1; $i < $sentence_length; $i++) {
	      $greeking .= " " . $dictionary[array_rand($dictionary)];
	    }

	    $greeking .= ". ";
	    $words -= $sentence_length;
	  }
  }
  else {
  	// use different method for titles
  	$title_length = $words;
  	$array = array();
  	for ($i = 0; $i < $words; $i++) {
  		$array[] = $dictionary[array_rand($dictionary)];
  	}
  	$greeking = ucwords(implode(' ', $array));
  }
  return $greeking;
}

function devel_get_users() {
  $users = array();
  $result = db_query("SELECT uid FROM {users}");
  while($user = db_fetch_object($result)){
    $users[] = $user->uid;
  }
  return $users;
}

function devel_get_nodes($nid = 0) {
  $nodes = array();
  $result = db_query("SELECT nid, type FROM {node} WHERE type IN ('story', 'blog', 'forum', 'page') AND comment = 2 AND nid > %d", $nid);
  while($node = db_fetch_object($result)){
    $nodes[$node->nid] = $node->type ;
  }
  return $nodes;
}

function devel_get_comments() {
  $comments = array();
  $result = db_query("SELECT nid, cid FROM {comments}");
  while($comment = db_fetch_object($result)){
    $comments[$comment->nid][] = $comment->cid ;
  }
  return comments;
}