<?php
// $Id: content_plugin_display_simple.inc,v 1.1.2.1 2008/09/04 18:12:27 yched Exp $

/**
 * Handler for 'content_simple' display.
 */
class content_plugin_display_simple extends views_plugin_display {
  function execute() {
    return $this->view->render($this->display->id);
  }

  function render() {
    return !empty($this->view->result) || !empty($this->view->style_plugin->definition['even empty']) ? $this->view->style_plugin->render($this->view->result) : '';
  }
}