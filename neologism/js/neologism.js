// declare Neologism namespace
var Neologism = {};

if( Drupal.jsEnabled ) {
  
  $(document).ready( function() {
    //$('#edit-field-literal-as-range-value').click(Neologism.checkRangeField);
    // check if the checkbox is checked is so, then hide rangeField show it otherwise
    //Neologism.checkRangeField();
    Neologism.checkResourceType();
    
    if ( $('#edit-field-custom-namespace-0-value').val() != '' ) {
    	$('#edit-namespace-type-2').attr('checked', true);
    	Neologism.neoVocabularyFormToggleNamespace();
	}

	console.info(Neologism);  //add observer to the domain and range widget
    
	if( Neologism.domainsTermsTree !== undefined && Neologism.inverseTermsTree !== undefined ) {
	  console.log('adding observer');
		Neologism.domainsTermsTree.addObserver(Neologism.inverseTermsTree);
	  console.log('Neologism.domainsTermsTree');
	  console.info(Neologism.domainsTermsTree);
	}
	
	if( Neologism.rangeTermsTree !== undefined && Neologism.inverseTermsTree !== undefined ) {
	  Neologism.rangeTermsTree.addObserver(Neologism.inverseTermsTree);
	  console.log('Neologism.rangeTermsTree');
	  console.info(Neologism.rangeTermsTree);
	}
	
	// this is used when all the content type form are shown the title field should take the focus
	$('#edit-title').focus();
	  
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
  };
  
  Neologism.checkResourceType = function() {
    // Another resource
	if ( $('#edit-resource-type-1').attr('checked') ) {
    	$('#range-group-datatypes').hide();
    	$('#range-treeview').show();
    	// the inverse selection widget should be shown if the range field also are shown
    	if( Neologism.inverseTermsTree !== undefined ) 
    		Neologism.inverseTermsTree.enable();
    	//$('#inverse-treeview').show();
    }
	// A literal (string, number, date, ...)
    else if ( $('#edit-resource-type-2').attr('checked') ) {
    	$('#range-treeview').hide();
    	$('#range-group-datatypes').show();
    	// the inverse selection widget should be hidden if the range field also are hidden
    	//$('#inverse-treeview').hide();
    	if( Neologism.inverseTermsTree !== undefined ) 
    		Neologism.inverseTermsTree.disable();
    }
	// Either
    else if ( $('#edit-resource-type-3').attr('checked') ) {
    	$('#range-group-datatypes').hide();
    	$('#range-treeview').hide();
    	if( Neologism.inverseTermsTree !== undefined ) 
    		Neologism.inverseTermsTree.enable();
    }
  };
  
  Neologism.neoVocabularyFormOnSubmit = function() {
	  if ( $('#edit-namespace-type-1').attr('checked') ) {
		  $('#edit-field-custom-namespace-0-value').val('');
	  } else {
		  // TODO: handle what happen when the user select custom namespace and the field it is empty
		  if ( $('#edit-field-custom-namespace-0-value').val() == '' ) {
			  $('#edit-field-custom-namespace-0-value').val('error_field_required_empty')
		  }
	  }
  };
  
  Neologism.neoVocabularyFormToggleNamespace = function() {
    // Another resource
	if ( $('#edit-namespace-type-1').attr('checked') ) {
		$('#edit-field-custom-namespace-0-value').attr('disabled', true);
    }
	// A literal (string, number, date, ...)
    else if ( $('#edit-namespace-type-2').attr('checked') ) {
    	$('#edit-field-custom-namespace-0-value').removeAttr("disabled").focus();//.val("editable now");
    }
  };
  
}
