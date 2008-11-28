// $Id: content.js,v 1.1 2008/03/02 23:18:08 yched Exp $

/**
 * Show the Save button when fields are swapped.
 *
 * This behavior is dependent on the tableDrag behavior, since it uses the
 * objects initialized in that behavior to update the row.
 */
Drupal.behaviors.cckFieldDrag = function(context) {
  var tableDrag = Drupal.tableDrag['content-field-overview']; // Get the tableDrag object.

  // Add a handler for when a row is dropped,
  // show Save button if the table has been changed.
  tableDrag.onDrop = function() {
    if (tableDrag.changed) {
      $('.content-admin-field-overview-submit').fadeIn('slow');
    }
  }
};

