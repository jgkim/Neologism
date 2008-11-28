$modulepath = drupal_get_path ('module', 'my_module');
  $cck_definition_file = $modulepath."/story_type.cck";

  $values['type_name'] = 'story';
  $values['macro'] = file_get_contents($cck_definition_file);

  include_once( drupal_get_path('module', 'node') .'/content_types.inc');
  include_once( drupal_get_path('module', 'content') .'/content_admin.inc');

  drupal_execute("content_copy_import_form", $values);
  
  
  
  
  
--------------------------------



function xxx_update_123() {
  include_once drupal_get_module('module', 'content') .'/content_crud.inc';
  content_field_instance_update(array('field_name' => 'field_layout', 'weight' => -2));
}


-----------------

/all.DRUPAL-5/cck/content_crud.inc:222:function content_field_instance_update($properties) {
./all.HEAD/cck/tests/content.crud.test:326:    $this->last_field = content_field_instance_update($settings);
./all.HEAD/imagefield/imagefield.install:154:    content_field_instance_update($field);


-----------------------------------
minumum fields definition for updating a CCK field

$w =  array (
    'type_name' => 'neo_class',
    'label' => ' scor6Superclass',
    'field_name' => 'field_subclassof',
    'type' => 'nodereference',
    'widget_type' => 'nodereference_select',
    'weight' => 0,
    'module' => 'nodereference',
    'widget_module' => 'nodereference',
  );

  include_once drupal_get_path('module', 'content') .'/includes/content.crud.inc';
  content_field_instance_update($w);





Fatal error: Cannot pass parameter 2 by reference in /Applications/MAMP/htdocs/dneo_svn6/install.php on line 1112