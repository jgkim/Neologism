<?php
/**
 * @file
 * Admin page callbacks for the Neologism module.
 */

function neologism_overview_diagram($node) {
   // show the diagram is there is at least one class or a property
  $base = base_path();
  $swf = $base . drupal_get_path('module','neologism').'/includes/Diagram.swf';
  $img_legend = $base . drupal_get_path('module','neologism').'/includes/legend.png';
  $edit_mode = TRUE;//(neologism_has_user_access($node, $user, NULL) && user_access('edit vocabularies'));
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
  
  return $output;
}
