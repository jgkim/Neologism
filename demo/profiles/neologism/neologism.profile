<?php
// $Id:  Exp $

/**
 * @file
 * Install Profile for Neologism 
 */
 
/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *  An array of modules to be enabled.
 */
function neologism_profile_modules() {
  return array(

    // Core - optional
    'color', 'help', 'menu', 'path', 'search', 'taxonomy',

    // Core - required
    'block', 'filter', 'node', 'system', 'user', 'watchdog',

    // CCK core
    'content', 'nodereference', 'optionwidgets', 'text', 'userreference', 'content_copy',
    
    // CCK
    'multiselect',
    
    // Neologism
    'neologism',
  );
}

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile.
 */
function neologism_profile_details() {
  return array(
    'name' => 'Neologism',
    'description' => 'Tool to publish RDF vocabularies online.'
  );
}

/**
 * Perform any final installation tasks for this profile.
 *
 * @return
 *   An optional HTML string to display to the user on the final installation
 *   screen.
 */
function neologism_profile_final() {
  // Site information
  variable_set('site_name', 'Neologism');
  variable_set('site_mission', 'Welcome to <strong>Neologism</strong>, an online editor to publish RDF vocabularies.');
  //variable_set('site_frontpage', 'node/add/class');
  
  // Menus
//   $item = array();
//   $item['pid'] = 2; // Primary items
//   $item['path'] = 'node/add/geshinode';
//   $item['title'] = 'Submit Code';
//   $item['description'] = 'Paste a code snippet.';
//   $item['weight'] = -6;
//   $item['type'] = 118;
//   menu_save_item($item); 
  
  // Permissions
//   db_query("UPDATE {permission} SET perm = '%s' WHERE rid = %d",
//     'create source code node, access content, clone node, search content', 1);
//   db_query("UPDATE {permission} SET perm = '%s' WHERE rid = %d",
//     'create source code node, edit own source code node, access content, clone node, search content', 2);
  
  // Taxonomy
//   $tags = array(
//     'name' => 'Tags',
//     'help' => 'Any tags you would like to associate with your code, delimitered by commas (example: Views, CCK, Module, etc).',
//     'relations' => '0',
//     'hierarchy' => '1',
//     'multiple' => '0',
//     'required' => '0',
//     'tags' => '1',
//     'module' => 'taxonomy',
//     'weight' => '0',
//     'nodes' => array('geshinode' => 'geshinode')
//   );
//   taxonomy_save_vocabulary($tags);
  
}
