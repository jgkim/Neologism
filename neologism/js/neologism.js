

if( Drupal.jsEnabled ) {
	
	// declare Neologism namespace
	var Neologism = {};
	
	$(document).ready( function() {
		// need for the Ext module
		Ext.QuickTips.init();
		
		// we need to check for the form and later ask for the rest
		if( Neologism.superclassesTreePanel !== undefined ) {
			Neologism.superclassesTreePanel.render(Neologism.superclassesTreePanel.objectToRender);
		}
		
		if( Neologism.disjointwithTreePanel !== undefined ) {
			Neologism.disjointwithTreePanel.render(Neologism.disjointwithTreePanel.objectToRender);
		}
		
		if( Neologism.domainTermsTree !== undefined ) {
			Neologism.domainTermsTree.render(Neologism.domainTermsTree.objectToRender);
		}
		
		if( Neologism.rangeTermsTree !== undefined ) {
			Neologism.rangeTermsTree.render(Neologism.rangeTermsTree.objectToRender);
		}
		
		if( Neologism.superpropertyTermsTree !== undefined ) {
			Neologism.superpropertyTermsTree.render(Neologism.superpropertyTermsTree.objectToRender);
		}
		
		if( Neologism.inverseTermsTree !== undefined ) {
			// if Neologism.domainsTermsTree is defined we are in the add/edit property form
			Neologism.domainTermsTree.addObserver(Neologism.inverseTermsTree);
			Neologism.rangeTermsTree.addObserver(Neologism.inverseTermsTree);
			
			Neologism.inverseTermsTree.render(Neologism.inverseTermsTree.objectToRender);
		}
		
		    //$('#edit-field-literal-as-range-value').click(Neologism.checkRangeField);
		// check if the checkbox is checked is so, then hide rangeField show it otherwise
		//Neologism.checkRangeField();
		Neologism.checkResourceType();
		
		if ( $('#edit-field-custom-namespace-0-value').val() != '' ) {
		$('#edit-namespace-type-2').attr('checked', true);
			Neologism.neoVocabularyFormToggleNamespace();
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
		}
		// A literal (string, number, date, ...)
		else if ( $('#edit-resource-type-2').attr('checked') ) {
			$('#range-treeview').hide();
			$('#range-group-datatypes').show();
			// the inverse selection widget should be hidden if the range field also are hidden
			Neologism.rangeTermsTree.clearValues();
		}
		// Either
		else if ( $('#edit-resource-type-3').attr('checked') ) {
			$('#range-group-datatypes').hide();
			$('#range-treeview').hide();
			Neologism.rangeTermsTree.clearValues();
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
