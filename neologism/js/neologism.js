// declare Neologism namespace
var Neologism = {};

if( Drupal.jsEnabled ) {
  
  $(document).ready( function() {
    $('#edit-field-literal-as-range-value').click(Neologism.checkRangeField);
    // check if the checkbox is checked is so, then hide rangeField show it otherwise
    Neologism.checkRangeField();
  }); // ready
  
  Neologism.checkRangeField = function() {
    var rangeField = $('#range-field');  
    var literalAsRangeCheckBox = $('#edit-field-literal-as-range-value');
    
    if( literalAsRangeCheckBox.is(':checked') ) { 
      rangeField.hide();
    }
    else {
      rangeField.show();
    }
  }
  
}
