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
    'color', 'help', 'menu', 
    'path', 
    'taxonomy', 'dblog',

    // Core - required
    'block', 'filter', 'node', 'system', 'user', 'trigger',

    // CCK core
    'content', 'nodereference', 'optionwidgets', 'text', 'userreference', 'content_copy', 'fieldgroup',
    
  	// rules module
    'rules',
  
    // Contrib
    'rdf', 
    //'sparql', 
    //'evoc', 
    //'evocreference', 'ext', 'mxcheckboxselect',
    
    // Neologism
    //'neologism',
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
    'description' => 'Neologism is a pre-packaged web site that lets users easily create and publish RDF vocabularies.'
  );
}

/**
 * Return a list of tasks that this profile supports.
 *
 * @return
 *   A keyed array of tasks the profile will perform during
 *   the final stage. The keys of the array will be used internally,
 *   while the values will be displayed to the user in the installer
 *   task list.
 */
function neologism_profile_task_list() {
	return array(
		'building-neologism-perspective' => st('Building Neologism\'s Perspective')
	);
}


/**
 * Perform any final installation tasks for this profile.
 *
 * @return
 *   An optional HTML string to display to the user on the final installation
 *   screen.
 */
function neologism_profile_tasks(&$task, $url) {
  if( $task == 'profile' ) {
	
		//module_rebuild_cache();
	  
	  // Insert default user-defined node types into the database. For a complete
	  // list of available node type attributes, refer to the node type API
	  // documentation at: http://api.drupal.org/api/HEAD/function/hook_node_info.
	  $types = array(
	    array(
	      'type' => 'page',
	      'name' => st('Page'),
	      'module' => 'node',
	      'description' => st("A <em>page</em>, similar in form to a <em>story</em>, is a simple method for creating and displaying information that rarely changes, such as an \"About us\" section of a website. By default, a <em>page</em> entry does not allow visitor comments and is not featured on the site's initial home page."),
	      'custom' => TRUE,
	      'modified' => TRUE,
	      'locked' => FALSE,
	      'help' => '',
	      'min_word_count' => '',
	    ),
	    array(
	      'type' => 'story',
	      'name' => st('Story'),
	      'module' => 'node',
	      'description' => st("A <em>story</em>, similar in form to a <em>page</em>, is ideal for creating and displaying content that informs or engages website visitors. Press releases, site announcements, and informal blog-like entries may all be created with a <em>story</em> entry. By default, a <em>story</em> entry is automatically featured on the site's initial home page, and provides the ability to post comments."),
	      'custom' => TRUE,
	      'modified' => TRUE,
	      'locked' => FALSE,
	      'help' => '',
	      'min_word_count' => '',
	    ),
	  );
	
	  foreach ($types as $type) {
	    $type = (object) _node_type_set_defaults($type);
	    node_type_save($type);
	  }
	
	  // Default page to not be promoted and have comments disabled.
	  variable_set('node_options_page', array('status'));
	  variable_set('comment_page', COMMENT_NODE_DISABLED);
	
	  // Don't display date and author information for page nodes by default.
	  $theme_settings = variable_get('theme_settings', array());
	  $theme_settings['toggle_node_info_page'] = FALSE;
	  variable_set('theme_settings', $theme_settings);
	
	  $modules_list = array(
	    'sparql',
	    'evoc', 
	    'evocreference', 'ext', 'mxcheckboxselect',
	    //'neologism'
	  );
	  
	  drupal_install_modules($modules_list);
	  drupal_install_modules(array('neologism'));
	  
	  // Update the menu router information.
	  menu_rebuild();
	  
	  $task = 'building-neologism-perspective';
  }
  
  if( $task == 'building-neologism-perspective' ) {
  	// set the default ExtJS library path to same place where is located the ext module
  	variable_set('ext_path', drupal_get_path('module', 'ext') .'/ext-3.0.0');
  	
  	// disabled the user login block
  	db_query('update {blocks} set status = 0 where bid = 1' /*module = "user" and delta = 0'*/);
  	// move the navegation block to right region
  	db_query('update {blocks} set region = "right" where bid = 2' /*module = "user" and delta = 1'*/);

  	
  	require_once 'modules/block/block.admin.inc';
  	require_once 'modules/block/block.module';
  	
  	// create custom block, this kind of block are stored in the boxes table
  	$form_id = 'block_add_block_form';
  	$form_state['values'] = array(
			'module' => 'block',
  		'title' => '',
  		'info' => 'Login Link',
  		'body' => 'Powered by <a href="http://neologism.deri.ie/" title="Powered by Neologism, an Ontology Editor based on Drupal.">Neologism</a> | <a href="user" title="User Login form or User Account">Login</a>'
  	);  	
  	// submit the form using these values
  	drupal_execute($form_id, $form_state);
  	
  	// enable the custom block
  	db_query(
  		'insert into {blocks} (module, delta, theme, status, weight, region, cache) 
  		values ("%s", %d, "%s", %d, %d, "%s", %d)',
  		'block', '1', 'garland', 1, -6, 'footer', BLOCK_NO_CACHE  		
  	);
  	
  	// change the User Registration settings to 
  	// Only site administrators can create new user accounts.
  	require_once 'modules/user/user.admin.inc';
  	
  	$form_id = 'user_admin_settings';
  	$form_state['values'] = array(
			'user_register' => '0'
  	);  	
  	// submit the form using these values
  	drupal_execute($form_id, $form_state);
  	
  	// Disabled the logo from the default theme. in this case Garland
  	require_once 'modules/system/system.admin.inc';
  	$form_id = 'system_theme_settings';
  	$form_state['values'] = array(
			'toggle_logo' => '0',
  		'toggle_mission' => '1',
  		'toggle_node_info_neo_class' => '0',
  		'toggle_node_info_neo_property' => '0',
  		'toggle_node_info_neo_vocabulary' => '0'
  	);  	
  	// submit the form using these values
  	drupal_execute($form_id, $form_state);
  	
  	// set the default mission
  	$form_id = 'system_site_information_settings';
  	$form_state['values'] = array(
			'site_mission' => 'Here should goes the mission\'s message (We are working in such a message...)'
  	);  	
  	// submit the form using these values
  	drupal_execute($form_id, $form_state);
  	
  	// Add the "vocabulary editor" Role
  	$rol = 'vocabulary editor';
  	db_query('insert into {role} (name) values ("%s")', $rol);
  	$res = db_fetch_object(db_query('select rid from {role} where name = "%s"', $rol));
  	
  	// To get more creative, start with a list of all permissions.
  	$raw_permissions = module_invoke('neologism','perm');
	  foreach ($raw_permissions as $perm) {
	    $permissions .= $perm.', '; 
	  }
	  
	  $raw_permissions = module_invoke('node','perm');
	  $i = 0;
  	for (; $i < count($raw_permissions) - 1; $i++ ) {
  		$permissions .= $raw_permissions[$i].', '; 	
  	}
	  $permissions .= $raw_permissions[$i];
  	
	  db_query('insert into {permission} (rid, perm) values (%d, "%s")', $res->rid, $permissions);
  	
	  // Add a triggered rules
  	require_once 'sites/all/modules/rules/rules_admin/rules_admin.export.inc';
  	$form_id = 'rules_admin_form_import';
  	// Redirect to homepage when user has logged in
  	$form_state['values'] = array(
			'import' => "array (
									  'rules' => 
									  array (
									    'rules_2' => 
									    array (
									      '#type' => 'rule',
									      '#set' => 'event_user_login',
									      '#label' => 'Redirect to homepage when user has logged in',
									      '#active' => 1,
									      '#weight' => '0',
									      '#categories' => 
									      array (
									      ),
									      '#status' => 'custom',
									      '#conditions' => 
									      array (
									      ),
									      '#actions' => 
									      array (
									        0 => 
									        array (
									          '#weight' => 0,
									          '#type' => 'action',
									          '#settings' => 
									          array (
									            'path' => '<front>',
									            'query' => '',
									            'fragment' => '',
									            'force' => 0,
									            'immediate' => 0,
									          ),
									          '#name' => 'rules_action_drupal_goto',
									          '#info' => 
									          array (
									            'label' => 'Page redirect',
									            'module' => 'System',
									            'eval input' => 
									            array (
									              0 => 'path',
									              1 => 'query',
									              2 => 'fragment',
									            ),
									          ),
									        ),
									      ),
									      '#version' => 6003,
									    ),
									  ),
									)"
  	);  	
  	// submit the form using these values
  	drupal_execute($form_id, $form_state);
  	
  	//New users become vocabulary editors
  	$form_state['values'] = array(
			'import' => "array (
										  'rules' => 
										  array (
										    'rules_1' => 
										    array (
										      '#type' => 'rule',
										      '#set' => 'event_user_insert',
										      '#label' => 'New users become vocabulary editors',
										      '#active' => 1,
										      '#weight' => '0',
										      '#categories' => 
										      array (
										      ),
										      '#status' => 'custom',
										      '#conditions' => 
										      array (
										      ),
										      '#actions' => 
										      array (
										        0 => 
										        array (
										          '#weight' => 0,
										          '#info' => 
										          array (
										            'label' => 'Add user role',
										            'arguments' => 
										            array (
										              'user' => 
										              array (
										                'type' => 'user',
										                'label' => 'User whos roles should be changed',
										              ),
										            ),
										            'module' => 'User',
										          ),
										          '#name' => 'rules_action_user_addrole',
										          '#settings' => 
										          array (
										            'roles' => 
										            array (
										              0 => 3,
										            ),
										            '#argument map' => 
										            array (
										              'user' => 'account',
										            ),
										          ),
										          '#type' => 'action',
										        ),
										      ),
										      '#version' => 6003,
										    ),
										  ),
										)"
  	);
  	drupal_execute($form_id, $form_state);
  	
  	// hidden the SPARQL links
  	db_query('update {menu_links} set hidden = "1", customized = "1" where link_path = "sparql"');
  	db_query('update {menu_links} set hidden = "1", customized = "1" where link_path = "node/add/sparql"');
  	
  	// return control to the installer
	  $task = 'profile-finished';
  }
}
