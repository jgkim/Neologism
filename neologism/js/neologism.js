Drupal.behaviors.neologism = function (context) {
  if (!Drupal.myNeologism) {
    Drupal.myNeologism = new Drupal.neologism();
  }
  else {
    Drupal.myNeologism.bindForms();
  }
}

Drupal.neologism = function() {
  var self = this;
  this.prefix = 'neologism-';
  this.data = Drupal.settings.neologism;
  this.init();
  // if there are any of the Neologism forms then create the rules and bind them.
  if (typeof this.data !== "undefined" && typeof this.data.forms !== "undefined") {
	  this.forms = this.data['forms'];
	  this.validators = {};
	  this.addExtraRules();
	  this.bindForms();
  }
};

Drupal.neologism.prototype.init = function() {
  Ext.QuickTips.init();
	
	// we need to check for the form and later ask for the rest
	if ( Neologism.superclassesTreePanel !== undefined ) {
		Neologism.superclassesTreePanel.render(Neologism.superclassesTreePanel.objectToRender);
	}
	
	if ( Neologism.disjointwithTreePanel !== undefined ) {
		Neologism.disjointwithTreePanel.render(Neologism.disjointwithTreePanel.objectToRender);
	}
	
	if ( Neologism.domainTermsTree !== undefined ) {
		Neologism.domainTermsTree.render(Neologism.domainTermsTree.objectToRender);
	}
	
	if ( Neologism.rangeTermsTree !== undefined ) {
		Neologism.rangeTermsTree.render(Neologism.rangeTermsTree.objectToRender);
	}
	
	if ( Neologism.superpropertyTermsTree !== undefined ) {
		Neologism.superpropertyTermsTree.render(Neologism.superpropertyTermsTree.objectToRender);
	}
	
	if ( Neologism.inverseTermsTree !== undefined ) {
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
	Drupal.neologism.checkResourceType();
	
    // Prepare custom namespace selection widget
    // Move custom namespace edit field next to the "Custom" radio button
    $('#edit-namespace-1-wrapper').append($('#edit-custom-namespace'));
    $('#edit-custom-namespace-wrapper').remove();
    // Enable and disable the custom namespace field as required
    if ($('#edit-namespace-0').attr('checked')) {
        $('#edit-custom-namespace').attr('disabled', true);
    }
    $('#edit-namespace-0').click(function() {
        $('#edit-custom-namespace').attr('disabled', true);
    });
    $('#edit-namespace-1').click(function() {
        $('#edit-custom-namespace').attr('disabled', false);
    });
    // Update the default namespace URI with the vocabulary ID
    setInterval(function() {
        var vocabID = $('#edit-prefix').val();
        if (vocabID) {
          $('#neologism-default-ns').empty().text(vocabID);
        } else {
          $('#neologism-default-ns').empty().html('<em>vocabulary-id</em>');
        }
    }, 250);
	
	// this is used when all the content type form are shown the title field should take the focus
	$('#edit-prefix').focus();
}

Drupal.neologism.checkResourceType = function() {
	// Another resource
	if ( $('#edit-resource-type-1').attr('checked') ) {
		$('#range-group-datatypes').hide();
		$('#range-treeview').show();
		// show inverse widget
		$('#inverse-panel').show();
		$('#superproperty-panel').removeClass('full-size-panel').addClass('half-size-panel');
	}
	// A literal (string, number, date, ...)
	else if ( $('#edit-resource-type-2').attr('checked') ) {
		$('#range-treeview').hide();
		$('#range-group-datatypes').show();
		// the inverse selection widget should be hidden if the range field also are hidden
		Neologism.rangeTermsTree.clearValues();
		$('#inverse-panel').hide();
		$('#superproperty-panel').removeClass('half-size-panel').addClass('full-size-panel');
	}
	// Either
	else if ( $('#edit-resource-type-3').attr('checked') ) {
		$('#range-group-datatypes').hide();
		$('#range-treeview').hide();
		Neologism.rangeTermsTree.clearValues();
		$('#inverse-panel').hide();
		$('#superproperty-panel').removeClass('half-size-panel').addClass('full-size-panel');
    }
	
	var propertyTreePanel = Ext.getCmp('ext-comp-1001');
	if (propertyTreePanel) {
		propertyTreePanel.syncSize();
	}
}

Drupal.neologism.checkTreeViewsHeight = function(object) {
  if (object.name == 'classesTreeViewPanel') {
	  Neologism.ctpHeight = object.newHeight;
  }
  else if (object.name == 'propertiesTreeViewPanel') {
	  Neologism.ptpHeight = object.newHeight;
  }
  
  if (typeof Neologism.ctpHeight !== 'undefined' && typeof Neologism.ptpHeight !== 'undefined') {
	  var maxHeight = (Neologism.ctpHeight > Neologism.ptpHeight) ? Neologism.ctpHeight : Neologism.ptpHeight; 
	  if (maxHeight > 500) {
		  // max height for both treeviews
		  maxHeight = 473;
	  }
	  $('#class-tree .x-panel-body').css({height:maxHeight});
	  $('#object-property-tree .x-panel-body').css({height:maxHeight});
  }
}

Drupal.neologism.prototype.bindForms = function(){
  var self = this;
  jQuery.each (self.forms, function(f) {
    // Add error container above the form, first look for standard message container
    var errorel = self.prefix + f + '-errors';
    if ($('div.messages').length) {
      if ($('div.messages').attr('id').length) {
        errorel = $('div.messages').attr('id');
      }
      else {
        $('div.messages').attr('id', errorel);
      }
    }
    else if (!$('#' + errorel).length) {
      $('<div id="' + errorel + '" class="messages error clientside-error"><ul></ul></div>').insertBefore('#' + f).hide();
    }
    
    // Remove any existing validation stuff
    if (self.validators[f]) {
      // Doesn't work :: $('#' + f).rules('remove');
      var form = $('#' + f).get(0); 
      jQuery.removeData(form, 'validator');
    }
    
    // Add basic settings
    self.validators[f] = $('#' + f).validate({
    ignore: ':hidden',
    errorClass: 'error',
    errorContainer: '#' + errorel,
    errorLabelContainer: '#' + errorel + ' ul',
    wrapper: 'li'
    });
    
    // Bind all rules
    self.bindRules(f);
  });
}

Drupal.neologism.prototype.bindRules = function(formid){
  var self = this;
  if ('rules' in self.forms[formid]){
    jQuery.each (self.forms[formid]['rules'], function(r) {
      // Check if element exist in DOM before adding the rule
      if ($("#" + formid + " :input[name='" + r + "']").length) {
        $("#" + formid + " :input[name='" + r + "']").rules("add", self.forms[formid]['rules'][r]);
      }
    });
  }
}

Drupal.neologism.prototype.addExtraRules = function(){
  jQuery.validator.addMethod("idValidator", function(value, element) { 
    return this.optional(element) || /^[a-z_]+[a-z0-9._-]*$/.test(value);
  }, jQuery.format('Please enter a valid identifier. It should start with letters. Only lowercase letters, numbers, dash and underscore. Max 10 characters.'));
  
  jQuery.validator.addMethod("idFlexibleValidator", function(value, element) { 
	return this.optional(element) || /^[a-zA-Z_]+[a-zA-Z0-9._-]*$/.test(value);
  }, jQuery.format('Please enter a valid identifier. It should start with letters or underscore. Only letters, numbers, dash and underscore. Max 10 characters.'));
  
  
  jQuery.validator.addMethod("uri_ending", function(value, element) {
  return this.optional(element) || /(\/$|#$)/.test(value);
  }, jQuery.format("The custom namespace URI must end in \"#\"or \"/\""));
}


