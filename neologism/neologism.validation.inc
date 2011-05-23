<?php
/**
 * Regular form + CCK
 */
function neologism_form_after_build(&$form, &$form_state) {
  static $js_rules = array();
  
  neologism_form_after_build_recurse($form['#id'], $form, $form_state, $js_rules);
  if (!empty($js_rules)) {
    $settings['neologism']['general'] = array(
      "errorClass" => "error",
      "wrapper" => "li",
    );
    $settings['neologism']['forms'][$form['#id']]['settings'] = array(
      "errorContainer" => "#formerrors-" . $form['#id'],
      "errorLabelContainer" => "#formerrors-" . $form['#id'] . " ul",
    );
    foreach ($js_rules as $key => $rule) {
      $settings['neologism']['forms'][$form['#id']]['rules'][$key] = $rule;
    }
    
    drupal_add_js($settings, 'setting');
  }
  return $form;
}

function neologism_form_after_build_recurse($form_id, &$form, &$form_state, &$js_rules) {
  if ($children = array_values(element_children($form))) {
    foreach ($children as $index => $item) {
      $element = $form[$item];
      if ((isset($element['#type']) && isset($element['#field_name'])) || (isset($element['#type']) && isset($form_state['#field_info'][$element['#parents'][0]]))) {
        neologism_cck($form_id, $element, $js_rules);
      }
      else {
        $types = array(
          'textfield', 'textarea', 'select', 'radio', 'checkbox', 'password', 'file', 'radios', 'checkboxes',
        );
        if (isset($element['#type']) && in_array($element['#type'], $types)) {
          neologism_regular($form_id, $element, $js_rules);
        }
      }
      neologism_form_after_build_recurse($form_id, $element, $form_state, $js_rules);
    }
  }
}

function neologism_cck($form_id, $element, &$js_rules) {
  
  static $multiples = array();
  if (isset($element['#name']) && !isset($js_rules[$element['#name']])) {
    if (!isset($element['#field_name'])) {
      $element['#field_name'] = $element['#parents'][0]; // Is this a good idea???
    }
    $cckfield = content_fields($element['#field_name']);
    $el_name = $element['#name'];
    $is_multiple = $cckfield['multiple'];
    $title = t($cckfield['widget']['label']);
    
    switch ($cckfield['type']) {
      case 'text':
        switch ($element['#type']) {
          case 'textfield':
          case 'textarea':
            if ($is_multiple) {
              // Only first field is required
              if (!isset($multiples[$form_id][$cckfield['field_name']])) {
                _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
                $multiples[$form_id][$cckfield['field_name']] = 1;
              }
            }
            else {
              _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            }
            if (isset($cckfield['max_length']) && $cckfield['max_length'] > 0) {
              _neologism_set_minmaxlength($el_name, $element['#title'], '', $cckfield['max_length'], $js_rules);
            }
            break;
          case 'select':
            if ($is_multiple) {
              $el_name .= '[]';
              _neologism_set_minmaxlength($el_name, $element['#title'], '', $is_multiple, $js_rules);
            }
            _neologism_set_required($el_name, $element['#title'], $cckfield['required'], $js_rules);
            break;
          case 'radio':
            _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            break;
          case 'checkbox':
            if ($is_multiple) {
              // We don't have a parent element to connect to, so no go, outer div has only a class
              // The checkboxes element has the unique name, but this isn't added to the outer div
              $id = '#edit-' . str_replace('_', '-', $element['#parents'][0]) . '-value-wrapper';
              /*if (!isset($multiples[$form_id][$element['#field_name']])) {
                _neologism_set_checkboxgroup_minmax($el_name, $cckfield['widget']['label'], $id, 0, $js_rules);
                $multiples[$form_id][$element['#field_name']] = 1;
              }
              else {
                _neologism_set_checkboxgroup_minmax($el_name, $cckfield['widget']['label'], $id, 1, $js_rules);
              }*/
              _neologism_set_checkboxgroup_minmax($element['#name'], $element['#title'], $id, 0, $js_rules);
            }
            else {
              _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            }
            break;
        }
        break;
      case 'number_decimal':
      case 'number_float':
      case 'number_integer':
        switch ($element['#type']) {
          case 'textfield':
            if ($is_multiple) {
              // Only first field is required
              if (!isset($multiples[$form_id][$cckfield['field_name']])) {
                _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
                $multiples[$form_id][$cckfield['field_name']] = 1;
              }
            }
            else {
              _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            }
            if ($cckfield['type'] == 'number_integer') {
              _neologism_set_number($el_name, $title, $cckfield['decimal'], $js_rules);
            }
            else {
              _neologism_set_number_decimal($el_name, $title, $cckfield['decimal'], $js_rules);
            }
            _neologism_set_minmax($el_name, $title, $cckfield['min'], $cckfield['max'], $js_rules);
            break;
          case 'select':
            if ($is_multiple) {
              $el_name .= '[]';
              _neologism_set_minmaxlength($el_name, $element['#title'], '', $is_multiple, $js_rules);
            }
            _neologism_set_required($el_name, $element['#title'], $cckfield['required'], $js_rules);
            _neologism_set_minmax($el_name, $element['#title'], $cckfield['min'], $cckfield['max'], $js_rules);
            break;
          case 'radio':
            _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            break;
          case 'checkbox':
            if ($is_multiple) {
              // We don't have a parent element to connect to, so no go, outer div has only a class
              // The checkboxes element has the unique name, but this isn't added to the outer div
              $id = '#edit-' . str_replace('_', '-', $element['#parents'][0]) . '-value-wrapper';
              /*if (!isset($multiples[$form_id][$element['#field_name']])) {
                _neologism_set_checkboxgroup_minmax($el_name, $cckfield['widget']['label'], $id, 0, $js_rules);
                $multiples[$form_id][$element['#field_name']] = 1;
              }
              else {
                _neologism_set_checkboxgroup_minmax($el_name, $cckfield['widget']['label'], $id, 1, $js_rules);
              }*/
              _neologism_set_checkboxgroup_minmax($element['#name'], $element['#title'], $id, 0, $js_rules);
            }
            else {
              _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            }
            break;
        }
        break;
      case 'email':
        $el_name = $element['email']['#name'];
        _neologism_set_email($el_name, $title, $js_rules);
        break;
      case 'filefield': // files[field_file1_0]
        switch ($element['#type']) {
          case 'filefield_widget':
          case 'imagefield_widget':
            $el_name = $element['upload']['#name'];
              if ($is_multiple) {
                // Only first field is required
                if (!isset($multiples[$form_id][$cckfield['field_name']])) {
                  _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
                  $multiples[$form_id][$cckfield['field_name']] = 1;
                }
              }
              else {
                _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
              }
              if (isset($cckfield['widget']['file_extensions'])) {
                $extensions = explode(' ', $cckfield['widget']['file_extensions']);
                _neologism_set_extensions($el_name, $extensions, $js_rules);
              }
            break;
        }
        break;
      case 'date':
        switch ($element['#type']) {
          case 'select':
            if (empty($element['#title'])) {
              // Is the case when the labels are inside the select box :/
              $title = t(ucfirst(array_pop(explode('-', $element['#id']))));
            }
            _neologism_set_required($el_name, $title, $cckfield['required'], $js_rules);
            break;
        }
        break;
    }
  }
}

function neologism_regular($form_id, $element, &$js_rules) {
  static $multiples = array();
  if (isset($element['#name']) && !isset($js_rules[$element['#name']])) {
    $el_name = $element['#name'];
    $el_title = $el_name;
    if (isset($element['#title'])) {
      $el_title = $element['#title'];
    }
    $is_multiple = FALSE;
    if (isset($element['#multiple'])) {
      $is_multiple = $element['#multiple'];
    }
    switch ($element['#type']) {
      case 'textfield':
      case 'password':
      case 'textarea':
      case 'file':
        if ($is_multiple) {
          // Only first field is required
          if (!isset($multiples[$form_id][$el_name])) {
            _neologism_set_required($el_name, $el_title, $element['#required'], $js_rules);
            $multiples[$form_id][$el_name] = 1;
          }
        }
        else {
          _neologism_set_required($el_name, $el_title, $element['#required'], $js_rules);
        }
        if ((isset($element['#maxlength']) && $element['#maxlength'] > 0) || (isset($element['#minlength']))) {
          _neologism_set_minmaxlength($el_name, $el_title, isset($element['#minlength']) ? $element['#minlength'] : '', $element['#maxlength'], $js_rules);
        }
        
        if (!empty($element['#neologism_validation_methods'])) {
          $message = (!empty($element['#neologism_validation_message'])) ? $element['#neologism_validation_message'] : '';
          foreach ($element['#neologism_validation_methods'] as $method) {
            _neologism_set_validation_method($el_name, $el_title, $method, $message, $js_rules);  
          }
        }
        
        break;
      case 'select':
        if ($is_multiple) {
          $el_name .= '[]';
          if (!isset($element['#minlength'])) {
            $element['#minlength'] = 0;
          }
          _neologism_set_minmaxlength($el_name, $el_title, $element['#minlength'], $element['#maxlength'], $js_rules);
        }
        _neologism_set_required($el_name, $el_title, $element['#required'], $js_rules);
        break;
      case 'radio':
      case 'radios':
        _neologism_set_required($el_name, $el_title, $element['#required'], $js_rules);
        break;
      case 'checkbox':
      case 'checkboxes':
        $is_multiple = (isset($element['#options']) && count($element['#options']>1));
        if ($is_multiple && $element['#required']) {
          $count = 0;
          foreach ($element['#options'] as $key => $value) {
            $id = '#edit-' . str_replace('_', '-', $element['#parents'][0]);
            _neologism_set_checkboxgroup_minmax($element[$key]['#name'], $el_title, $id, $count, $js_rules);
            $count++;
          }
        }
        else if ($element['#required']) {
          _neologism_set_required($el_name, $el_title, $element['#required'], $js_rules);
        }
        break;
    }
  }
}

/**
 * Set validation rule for required fields.
 */
function _neologism_set_required($name, $title, $required, &$js_rules, $message = '') {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  if ($required) {
    $js_rules[$name]['required'] = TRUE;
    $js_rules[$name]['messages']['required'] = (empty($message)) ? t('!name field is required.', array('!name' => $title)) : $message;
  }
}

/**
 * Set validation rule for number fields.
 */
function _neologism_set_number($name, $title, $decimalpoint, &$js_rules) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $js_rules[$name]['digits_negative'] = TRUE;
  $js_rules[$name]['messages']['digits_negative'] = t('!name field accepts only numbers.', array('!name' => $title));
}

/**
 * Set validation rule for decimal fields.
 */
function _neologism_set_number_decimal($name, $title, $decimalpoint, &$js_rules) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  if ($decimalpoint == '.') {
    $js_rules[$name]['number'] = TRUE;
    $js_rules[$name]['messages']['number'] = t('!name field accepts only numbers (use a \'.\' as decimal point).', array('!name' => $title));
  }
  else {
    $js_rules[$name]['numberDE'] = TRUE;
    $js_rules[$name]['messages']['numberDE'] = t('!name field accepts only numbers (use a \',\' as decimal point).', array('!name' => $title));
  }
}

/**
 * Set validation rule for fields with a minimum and/or a maximum value.
 */
function _neologism_set_minmax($name, $title, $min, $max, &$js_rules, $message = '') {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  if (isset($min) && $min != '' && isset($max) && $max != '') {
    $js_rules[$name]['range'] = array($min, $max);
    $js_rules[$name]['messages']['range'] = (empty($message)) ? t('!name field has to be between !min and !max.', array('!name' => $title, '!min' => $min, '!max' => $max)) : $message;
  }
  elseif (isset($min) && $min != '') {
    $js_rules[$name]['min'] = $min;
    $js_rules[$name]['messages']['min'] = (empty($message)) ? t('!name field has to be greater than !min.', array('!name' => $title, '!min' => $min)) : $message;
  }
  elseif (isset($max) && $max != '') {
    $js_rules[$name]['max'] = $max;
    $js_rules[$name]['messages']['max'] = (empty($message)) ? t('!name field has to be smaller than !max.', array('!name' => $title, '!max' => $max)) : $message;
  }
}

/**
 * Set validation rule for fields with a minimum and/or maximum length.
 */
function _neologism_set_minmaxlength($name, $title, $min, $max, &$js_rules, $message = '') {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  if (isset($min) && $min != '' && isset($max) && $max != '') {
    $js_rules[$name]['rangelength'] = array($min, $max);
    $js_rules[$name]['messages']['rangelength'] = (empty($message)) ? t('!name field has to have between !min and !max values.', array('!name' => $title, '!min' => $min, '!max' => $max)) : $message;
  }
  elseif (isset($min) && $min != '') {
    $js_rules[$name]['minlength'] = $min;
    $js_rules[$name]['messages']['minlength'] = (empty($message)) ? t('!name field has to have minimal !min values.', array('!name' => $title, '!min' => $min)) : $message;
  }
  elseif (isset($max) && $max != '') {
    $js_rules[$name]['maxlength'] = $max;
    $js_rules[$name]['messages']['maxlength'] = (empty($message)) ? t('!name field has to have maximum !max values.', array('!name' => $title, '!max' => $max)) : $message;
  }
}

/**
 * Set validation rule for required fields that must equal a value from an other field.
 */
function _neologism_set_equal($name, $title, $value, &$js_rules) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $js_rules[$name]['equalTo'] = ':input[name=\'submitted[' . $value['form_key'] . ']\']';
  $js_rules[$name]['messages']['equalTo'] = t('!name field has to be equal to !firstone.', array('!name' => $title, '!firstone' => $value['name']));
}

/**
 * Set validation rule for fields that can not be equal to a value from an other field.
 */
function _neologism_set_not_equal($name, $title, $value, &$js_rules, $message = '') {
  $js_rules[$name]['notEqualTo'] = ':input[name=\'submitted[' . $value['form_key'] . ']\']';
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $message = empty($message) ? t('!name field has to different from !firstone.', array('!name' => $title, '!firstone' => $value['name'])) : $message;
  $js_rules[$name]['messages']['notEqualTo'] = $message;
}

/**
 * Set validation rule for fields that must be equal to a specific value.
 */
function _neologism_set_specific_value($name, $title, $value, &$js_rules, $message = '') {
  $js_rules[$name]['oneOf'] = $value;
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $message = empty($message) ? t('!name field has to different from !firstone.', array('!name' => $title, '!firstone' => $value['name'])) : $message;
  $js_rules[$name]['messages']['oneOf'] = $message;
}

/**
 * Set validation rule for ean number fields.
 */
function _neologism_set_ean($name, $title, &$js_rules, $message = '') {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $message = empty($message) ? t('!name field is not a valid EAN number.', array('!name' => $title)) : $message;
  $js_rules[$name]['validEAN'] = TRUE;
  $js_rules[$name]['messages']['validEAN'] = $message;
}

/**
 * Set validation rule for file fields that must have a certain extension.
 */
function _neologism_set_extensions($name, $extensions, &$js_rules) {
  $extension_list = preg_replace('#,(?![^,]+,)#', ' or', implode(', ', $extensions));
  $js_rules[$name]['accept'] = implode('|', $extensions);
  $js_rules[$name]['messages']['accept'] = t("Only files with a %exts extension are allowed.", array('%exts' => $extension_list));
}

/**
 * Set validation rule for checkboxes.
 */
function _neologism_set_checkboxgroup_minmax($name, $title, $id, $count, &$js_rules, $min = 1, $max = 99) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $js_rules[$name]['checkboxgroupminmax'] = array($min, $max, $id);
  if ($count == 0) {
    $js_rules[$name]['messages']['checkboxgroupminmax'] = t('!name field is required.', array('!name' => $title));
  }
  else {
    $js_rules[$name]['messages']['checkboxgroupminmax'] = FALSE;
  }
}

/**
 * Set validation rule for email fields.
 */
function _neologism_set_email($name, $title, &$js_rules) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $js_rules[$name]['email'] = TRUE;
  $js_rules[$name]['messages']['email'] = t('The value in !name is not a valid email address.', array('!name' => $title));
}

function _neologism_set_validation_method($name, $title, $validation_method, $message, &$js_rules) {
  $title = variable_get('neologism_validation_prefix', '') . $title .variable_get('neologism_validation_suffix', '');
  $js_rules[$name][$validation_method] = TRUE;
  if ($message) {
    $js_rules[$name]['messages'][$validation_method] = $message;
  }
}