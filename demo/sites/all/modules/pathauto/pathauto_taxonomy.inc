<?php
// $Id: pathauto_taxonomy.inc,v 1.20.4.20 2007/12/16 21:57:13 greggles Exp $

/*
 * Implementation of hook_pathauto() for taxonomy module
 */
function taxonomy_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'taxonomy';
      $settings['token_type'] = 'taxonomy';
      $settings['groupheader'] = t('Category path settings');
      $settings['patterndescr'] = t('Default path pattern (applies to all vocabularies with blank patterns below)');
      $settings['patterndefault'] = t('category/[vocab-raw]/[catpath-raw]');
      $patterns = token_get_list('taxonomy');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['supportsfeeds'] = '0/feed';
      $settings['bulkname'] = t('Bulk generate aliases for categories that are not aliased');
      $settings['bulkdescr'] = t('Generate aliases for all existing categories which do not already have aliases.');

      $vocabularies = taxonomy_get_vocabularies();
      if (sizeof($vocabularies) > 0) {
        $settings['patternitems'] = array();
        $forum_vid = variable_get('forum_nav_vocabulary', '');
        foreach ($vocabularies as $vocab) {
          if ($vocab->vid != $forum_vid) {
            $vocabname = $vocab->name;
            $fieldlabel = t('Pattern for all %vocab-name paths', array('%vocab-name' => $vocabname));
            $settings['patternitems'][$vocab->vid] = $fieldlabel;
          }
        }
      }
      return (object) $settings;
    default:
      break;
  }
}

/**
 * Generate aliases for all categories without aliases
 * 
 */ 
function taxonomy_pathauto_bulkupdate() {
  $forum_vid = variable_get('forum_nav_vocabulary', '');
  $query = "SELECT tid, vid, name, src, dst FROM {term_data} LEFT JOIN {url_alias} ON CONCAT('taxonomy/term/', tid) = src WHERE src IS NULL AND vid != %d";
  $result = db_query_range($query, $forum_vid, 0, variable_get('pathauto_max_bulk_update', 50));
  
  $count = 0;
  $placeholders = array();
  while ($category = db_fetch_object($result)) {
    $count += _taxonomy_pathauto_alias($category, 'bulkupdate');
  }
  
  drupal_set_message(format_plural($count,
    "Bulk generation of terms completed, one alias generated.",
    "Bulk generation of terms completed, @count aliases generated."));
}

/**
 * 
 * Function to create aliases for taxonomy objects
 * 
 * @param object $category a taxonomy object
 * 
 */
function _taxonomy_pathauto_alias($category, $op) {
  $count = 0;

  $placeholders = pathauto_get_placeholders('taxonomy', $category);    
  
  $forum_vid = variable_get('forum_nav_vocabulary', '');
  // If we're in a forum vocabulary, also create a forum container, forum, or forum topic alias.
  if (module_exists('forum') && $forum_vid == (int)$category->vid) {
    $src = 'forum/'. $category->tid;
    if ($alias = pathauto_create_alias('forum', $op, $placeholders, $src, $category->tid, $vid)) {
      $count++;
    }
  }
  else {
    $src = 'taxonomy/term/'. $category->tid;
    if ($alias = pathauto_create_alias('taxonomy', $op, $placeholders, $src, $category->tid, $category->vid)) {
      $count++;
    }
  }
  return $count;
}


/*
 * Implementation of hook_pathauto() for forum module
 */
function forum_pathauto($op) {
  switch ($op) {
    case 'settings':
      $settings = array();
      $settings['module'] = 'forum';
      $settings['token_type'] = 'taxonomy';
      $settings['groupheader'] = t('Forum path settings');
      $settings['patterndescr'] = t('Pattern for forums and forum containers');
      $settings['patterndefault'] = t('[vocab-raw]/[catpath-raw]');
      $patterns = token_get_list('taxonomy');
      foreach ($patterns as $type => $pattern_set) {
        if ($type != 'global') {
          foreach ($pattern_set as $pattern => $description) {
            $settings['placeholders']['['. $pattern .']'] = $description;
          }
        }
      }
      $settings['supportsfeeds'] = '0/feed';
      $settings['bulkname'] = t('Bulk generate forum paths');
      $settings['bulkdescr'] = t('Generate aliases for all existing forums and forum containers which do not already have aliases.');
      return (object) $settings;
    default:
      break;
  }
}


/**
 *  Generate aliases for all forums and forum containers without aliases
 */
function forum_pathauto_bulkupdate() {
  $forum_vid = variable_get('forum_nav_vocabulary', '');
  $query = "SELECT tid, vid, name, src, dst FROM {term_data} LEFT JOIN {url_alias} ON CONCAT('forum/', tid) = src WHERE vid = %d AND src IS NULL";
  $result = db_query_range($query, $forum_vid, 0, variable_get('pathauto_max_bulk_update', 50));

  $count = 0;
  $placeholders = array();
  while ($category = db_fetch_object($result)) {
    $count = _taxonomy_pathauto_alias($category, 'bulkupdate') + $count;
  }

  drupal_set_message(format_plural($count,
    "Bulk update of forums and forum containers completed, one alias generated.",
    "Bulk update of forums and forum containers completed, @count aliases generated."));
}