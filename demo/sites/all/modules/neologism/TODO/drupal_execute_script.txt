function base_create_content($type = '<create>', $macro = '', $file = '') {
  if(!module_exists("content_copy")){
    drupal_set_message('Programmatically creating CCK fields requires the Content Copy module. Exiting.');
    return;
  }
  $values = array();
  $values['type_name'] = $type;
  //get macro import data, prefer file first
  if($file){
    if(file_exists($file)){
      $values['macro'] = file_get_contents($file);
    }
    else{
      drupal_set_message('Unable to read input file for import. Exiting.');
      return;
    }
  }
  elseif($macro){
    $values['macro'] = $macro;
  }
 
  // convert macro to real PHP array and then check if type already exists
  eval($values['macro']);
  if (db_table_exists('content_type_'. $content['type']['type'])) return;
 
  //include required files
  include_once './'. drupal_get_path('module', 'node') .'/content_types.inc';
  include_once('./'. drupal_get_path('module', 'content') .'/content_admin.inc');
 
  $typestr  = var_export($content['type'], true);
  $groupsstr = var_export($content['groups'], true);
  $fieldsstr = var_export($content['fields'], true);
 
  // convert back to string representation of macro
  $values['macro'] = <<<CONTENT
  \$content['type'] = $typestr;    
  \$content['groups'] = $groupsstr;  
  \$content['fields'] = $fieldsstr;
CONTENT;
 
  //import content by executing content copy import form and passing macro
  drupal_execute("content_copy_import_form", $values);
}
