<?php
// $Id: token_node.inc,v 1.3.2.21 2007/10/28 16:10:41 greggles Exp $

/**
 * Implementation of hook_token_values()
 */
function node_token_values($type, $object = NULL, $options = array()) {
  $values = array();
  switch ($type) {
    case 'node':
      $node = $object;

      $values['nid']              = $node->nid;
      $values['type']             = $node->type;
      $values['type-name']        = node_get_types('name', $node->type);
      $values['title']            = check_plain($node->title);
      $values['title-raw']        = $node->title;
      $values['author-uid']       = $node->uid;
      $values['author-name']      = check_plain($node->name);
      $values['author-name-raw']  = $node->name;
      if (isset($node->created)) {
        $date = (int)$node->created;
        $values['yyyy']           = date('Y', $date);
        $values['yy']             = date('y', $date);
        $values['month']          = date('F', $date);
        $values['mon']            = date('M', $date);
        $values['mm']             = date('m', $date);
        $values['m']              = date('n', $date);
        $values['ww']             = date('W', $date);
        $values['date']           = date('N', $date);
        $values['day']            = date('l', $date);
        $values['ddd']            = date('D', $date);
        $values['dd']             = date('d', $date);
        $values['d']              = date('j', $date);
      }

      if (isset($node->changed)) {
        $date = (int)$node->changed;
        $values['mod-yyyy']       = date('Y', $date);
        $values['mod-yy']         = date('y', $date);
        $values['mod-month']      = date('F', $date);
        $values['mod-mon']        = date('M', $date);
        $values['mod-mm']         = date('m', $date);
        $values['mod-m']          = date('n', $date);
        $values['mod-ww']         = date('W', $date);
        $values['mod-date']       = date('N', $date);
        $values['mod-day']        = date('l', $date);
        $values['mod-ddd']        = date('D', $date);
        $values['mod-dd']         = date('d', $date);
        $values['mod-d']          = date('j', $date);
      }
      
      // Now get the menu related information.
      global $_menu;
      $trail = array();
      $trail_raw = array();
      $original_mid = token_menu_get_mid('node/'.$node->nid);
      
      $mid = $original_mid;
      while ($mid && $_menu['visible'][$mid] && $_menu['visible'][$mid]['pid'] != 0) {
        array_unshift($trail, check_plain($_menu['visible'][$mid]['title']));
        $mid = $_menu['visible'][$mid]['pid'];
      }

      // One more time, unfiltered
      $mid = $original_mid;
      while ($mid && $_menu['visible'][$mid] && $_menu['visible'][$mid]['pid'] != 0) {
        array_unshift($trail_raw, $_menu['visible'][$mid]['title']);
        $mid = $_menu['visible'][$mid]['pid'];
      }

      if (isset($trail)) {
        $values['menupath']     = implode('/', $trail);
        $values['menupath-raw'] = implode('/', $trail_raw);
        $values['menu']         = check_plain($_menu['visible'][$mid]['title']);
        $values['menu-raw']     = $_menu['visible'][$mid]['title'];
      }
      else {
        $values['menu']         = '';
        $values['menu-raw']     = '';
        $values['menupath']     = '';
        $values['menupath-raw'] = '';
      }

      // And now taxonomy, which is a bit more work. This code is adapted from
      // pathauto's handling code; it's intended for compatability with it.
      if (module_exists('taxonomy') && !empty($node->taxonomy) && is_array($node->taxonomy)) {
        foreach ($node->taxonomy as $term) {
          $original_term = $term;
          if ((object)$term) {
            // With freetagging it's somewhat hard to get the tid, vid, name values
            // Rather than duplicating taxonomy.module code here you should make sure your calling module
            // has a weight of at least 1 which will run after taxonomy has saved the data which allows us to 
            // pull it out of the db here.
            if (!isset($term->name) || !isset($term->tid)) {
              $vid = db_result(db_query("SELECT t.vid FROM {term_node} r INNER JOIN {term_data} t ON r.tid = t.tid INNER JOIN {vocabulary} v ON t.vid = v.vid WHERE r.nid = %d ORDER BY v.weight, t.weight, t.name LIMIT 1", $object->nid));
              $term = db_fetch_object(db_query("SELECT t.tid, t.name FROM {term_data} t INNER JOIN {term_node} r ON r.tid = t.tid WHERE t.vid = %d AND r.nid = %d ORDER BY weight LIMIT 1", $vid, $object->nid));
              $term->vid = $vid;
            }

            // Ok, if we still don't have a term name maybe this is a pre-taxonomy submit node
            // So if it's a number we can get data from it
            if(!isset($term->name) && is_array($original_term)) {
              $tid = array_shift($original_term);
              if (is_numeric($tid)) {
                $term = taxonomy_get_term($tid);
              }
            }
            $values['term'] = check_plain($term->name);
            $values['term-raw'] = $term->name;
            $values['term-id'] = $term->tid;
            $vid = $term->vid;

            if(!empty($vid)) {
              $vocabulary = taxonomy_get_vocabulary($vid);
              $values['vocab'] = check_plain($vocabulary->name);
              $values['vocab-raw'] = $vocabulary->name;
              $values['vocab-id'] = $vocabulary->vid;
            }

            // The 'catpath' (and 'cat') tokens have been removed, as they caused quite a bit of confusion,
            // and the catpath was a relatively expensive query when the taxonomy tree was deep.
            //
            // It existed only to provide forward-compatability with pathauto module, and
            // for most uses of token.module, it was a relatively useless token -- it exposed
            // a list of term names formatted as a URL/path string. Once pathauto supports
            // tokens, *it* should handle this catpath alias as it's the primary consumer.
            break;
          }
        }
      } 
      // It's possible to leave that block and still not have good data.  
      // So, we test for these and if not set, set them.
      if (!isset($values['term'])) {
        $values['term'] = '';
        $values['term-raw'] = '';
        $values['term-id'] = '';
        $values['vocab'] = '';
        $values['vocab-raw'] = '';
        $values['vocab-id'] = '';
      }

      break;
  }
  
  return $values;
}

/**
 * Implementation of hook_token_list()
 */
function node_token_list($type = 'all') {
  if ($type == 'node' || $type == 'all') {
    $tokens['node']['nid']            = t('Node ID');
    $tokens['node']['type']           = t('Node type');
    $tokens['node']['type-name']      = t('Node type (user-friendly version)');
    $tokens['node']['title']          = t('Node title');
    $tokens['node']['title-raw']      = t('Unfiltered node title. WARNING - raw user input.');

    $tokens['node']['author-uid']     = t("Node author's user id");
    $tokens['node']['author-name']    = t("Node author's user name");
    $tokens['node']['author-name-raw']    = t("Node author's user name. WARNING - raw user input.");

    $tokens['node']['term']           = t("Name of top taxonomy term");
    $tokens['node']['term-raw']       = t("Unfiltered name of top taxonomy term. WARNING - raw user input.");
    $tokens['node']['term-id']        = t("ID of top taxonomy term");
    $tokens['node']['vocab']          = t("Name of top term's vocabulary");
    $tokens['node']['vocab-raw']      = t("Unfiltered name of top term's vocabulary. WARNING - raw user input.");
    $tokens['node']['vocab-id']       = t("ID of top term's vocabulary");

    // Temporarily disabled -- see notes in node_token_values.
    // $tokens['node']['catpath']        = t("Full taxonomy tree for the topmost term");

    $tokens['node']['yyyy']           = t("Node creation year (four digit)");
    $tokens['node']['yy']             = t("Node creation year (two digit)");
    $tokens['node']['month']          = t("Node creation month (full word)");
    $tokens['node']['mon']            = t("Node creation month (abbreviated)");
    $tokens['node']['mm']             = t("Node creation month (two digit, zero padded)");
    $tokens['node']['m']              = t("Node creation month (one or two digit)");
    $tokens['node']['ww']             = t("Node creation week (two digit)");
    $tokens['node']['date']           = t("Node creation date (day of month)");
    $tokens['node']['day']            = t("Node creation day (full word)");
    $tokens['node']['ddd']            = t("Node creation day (abbreviation)");
    $tokens['node']['dd']             = t("Node creation day (two digit, zero-padded)");
    $tokens['node']['d']              = t("Node creation day (one or two digit)");
    $tokens['node']['mod-????']       = t('All tokens for node creation dates can also be used with with the "mod-" prefix; doing so will use the modification date rather than the creation date.');

    $tokens['node']['menu']           = t("The name of the menu the node belongs to.");
    $tokens['node']['menu-raw']       = t("The name of the menu the node belongs to. WARNING - raw user input.");
    $tokens['node']['menupath']       = t("The menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /.");
    $tokens['node']['menupath-raw']   = t("The unfiltered menu path (as reflected in the breadcrumb), not including Home or [menu]. Separated by /. WARNING - raw user input.");
    return $tokens;
  }
}

// Handle book nodes.
function book_token_values($type, $object = NULL, $options = array()) {
  if ($type == 'node') {
    $node = $object;

    $tokens = array();
    if ($node->parent) {
      $path = book_location($node);
      $tokens['book'] = check_plain($path[0]->title);
      $tokens['book-raw'] = $path[0]->title;
      $tokens['book_id'] = $node->parent;

      $bookhierarchy = book_location($node);
      $bookpath = '';
      $bookpath_raw = '';
      foreach ($bookhierarchy as $bookelement) {
        if ($bookpath == '') {
          $bookpath = check_plain($bookelement->title);
          $bookpath_raw = $bookelement->title;
        } 
        else {
          $bookpath = $bookpath . '/' . check_plain($bookelement->title);
          $bookpath_raw = $bookpath_raw . '/' . $bookelement->title;
        }
      }
      $tokens['bookpath'] = $bookpath;
      $tokens['bookpath-raw'] = $bookpath_raw;
    } 
    else {
      $tokens['book'] = '';
      $tokens['book_id'] = '';
      $tokens['bookpath'] = '';
      $tokens['bookpath-raw'] = '';
    }
    return $tokens;
  }
}

function book_token_list($type) {
  if ($type == 'node' || $type == 'all') {
    $list['book']['book']           = t("The title of the node's book parent.");
    $list['book']['book_id']        = t("The id of the node's book parent.");
    $list['book']['bookpath']       = t("The titles of all parents in the node's book hierarchy.");

    $list['book']['book-raw']       = t("The unfiltered title of the node's book parent. WARNING - raw user input.");
    $list['book']['bookpath-raw']   = t("The unfiltered titles of all parents in the node's book hierarchy. WARNING - raw user input.");

    return $list;
  }
}

/**
 * Check if mid/path is present in the menu.
 *
 * @param $in
 *   Numeric input is treated as a menu-id, strings as src-paths.
 * @return
 *   An existing mid, or 0 if none found.
 */
function token_menu_get_mid($in) {
  global $_menu;

  if (!is_numeric($in)) {
    if (isset($_menu['path index'][$in])) {
      $mid = $_menu['path index'][$in];
    }
    else {
      $mid = 0;
    }
  }
  else if (!isset($_menu['visible'][$in])) {
    $mid = 0;
  }

  // temporary paths would break much of this module
  if ($mid < 0) $mid = 0;

  return $mid;
}

