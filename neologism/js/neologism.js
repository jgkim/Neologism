

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
			
			// this implicit event fire is to get the rootNode in inverseTermsTree to handle empty values for domain and range
			Neologism.domainTermsTree.fireEvent('selectionchange', Neologism.domainTermsTree.rootNode);
			Neologism.inverseTermsTree.render(Neologism.inverseTermsTree.objectToRender);
		}
		
		    //$('#edit-field-literal-as-range-value').click(Neologism.checkRangeField);
		// check if the checkbox is checked is so, then hide rangeField show it otherwise
		//Neologism.checkRangeField();
		Neologism.checkResourceType();
		
        // Prepare custom namespace selection widget
        // Move custom namespace edit field next to the "Custom" radio button
        $('#edit-namespace-1-wrapper').append($('#edit-field-custom-namespace-0-value'));
        $('#edit-field-custom-namespace-0-value-wrapper').remove();
        // Enable and disable the custom namespace field as required
        if ($('#edit-namespace-0').attr('checked')) {
            $('#edit-field-custom-namespace-0-value').attr('disabled', true);
        }
        $('#edit-namespace-0').click(function() {
            $('#edit-field-custom-namespace-0-value').attr('disabled', true);
        });
        $('#edit-namespace-1').click(function() {
            $('#edit-field-custom-namespace-0-value').attr('disabled', false);
        });
        // Update the default namespace URI with the vocabulary ID
        setInterval(function() {
            var vocabID = $('#edit-title').val();
            if (vocabID) {
              $('#neologism-default-ns').empty().text(vocabID);
            } else {
              $('#neologism-default-ns').empty().html('<em>vocabulary-id</em>');
            }
        }, 250);
		
		// this is used when all the content type form are shown the title field should take the focus
		$('#edit-title').focus();
	}); // ready

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
}
