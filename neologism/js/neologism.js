// declare Neologism namespace
var Neologism = {};

if( Drupal.jsEnabled ) {
  
  $(document).ready( function() {
    //$('#edit-field-literal-as-range-value').click(Neologism.checkRangeField);
    // check if the checkbox is checked is so, then hide rangeField show it otherwise
    //Neologism.checkRangeField();
    Neologism.checkResourceType();
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
  },
  
  Neologism.checkResourceType = function() {
    // Another resource
	if ( $('#edit-resource-type-1').attr('checked') ) {
    	$('#range-group-datatypes').hide();
    	$('#range-group-fieldrange2').show();
    	// the inverse selection widget should be shown if the range field also are shown
    	Neologism.createInverseSelecctionWidget.termsTree.enable();
    	//$('#inverse-treeview').show();
    }
	// A literal (string, number, date, ...)
    else if ( $('#edit-resource-type-2').attr('checked') ) {
    	$('#range-group-fieldrange2').hide();
    	$('#range-group-datatypes').show();
    	// the inverse selection widget should be hidden if the range field also are hidden
    	//$('#inverse-treeview').hide();
    	Neologism.createInverseSelecctionWidget.termsTree.disable();
    }
	// Either
    else if ( $('#edit-resource-type-3').attr('checked') ) {
    	$('#range-group-datatypes').hide();
    	$('#range-group-fieldrange2').hide();
    	Neologism.createInverseSelecctionWidget.termsTree.enable();
    }
    
  };
  
}
