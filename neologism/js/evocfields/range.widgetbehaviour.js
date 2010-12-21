/**
 * @author guicec
 */

/**
 * Create the class selection widget behaviour for filed_superclass2 
 * 
 * @param {Object} field_name
 */
Neologism.createRangeSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];

  Neologism.rangeTermsTree = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Range'),
    disabled: false,
    arrayOfValues: baseParams.arrayOfValues,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,
      listeners: {
        // load : ( Object This, Object node, Object response )
        // Fires when the node has been successfuly loaded.
        // added event to refresh the checkbox from its parent 
        load: function(loader, node, response){
          	
			// check the first element of the baseParams.arrayOfValues if this is a literal then we need to clear it from the
	    	// list of value
    		if( Neologism.TermsTree.getXSDDatatype().indexOf(baseParams.arrayOfValues[0]) != -1 ) {
				baseParams.arrayOfValues.length = 0;
			}
    		
    		 // we need to create the reference to arrayOfValues eventhough the array reside in the loader object
    		 // for a better use. The reference in creation time it is not working.
    		 node.getOwnerTree().arrayOfValues = baseParams.arrayOfValues;
        }
      }
    }),
    
    root: new Ext.tree.AsyncTreeNode({
      text	: Drupal.t('Thing / Superclass'),
      id		: 'root',                  // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false
    }),
    
    listeners: {
      	// behaviour for on checkchange in Neologism.superclassesTree TreePanel object 
      	checkchange: function(node, checked) {
	  		node.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
	  		
	        if ( checked /*&& node.parentNode !== null*/ ) {
		        // add selection to array of values
        		if ( baseParams.arrayOfValues.indexOf(node.text) == -1 ) {
	            	baseParams.arrayOfValues.push(node.text);
	            }
	            
	    	} 
	        else {
	    		// if we are unchecked a checkbox
	    		for ( var i = 0, len = baseParams.arrayOfValues.length; i < len; i++ ) {
	    			if ( baseParams.arrayOfValues[i] == node.text ) {
	    				baseParams.arrayOfValues.splice(i, 1);
	    			}
	    		}
	        }
	        // fire the event to execute the onSelectionChange handler and notify to observers
	        //this.fireEvent('selectionchange', node);
  		} // checkchange  
	        
        ,expandnode: function( node ) {
			node.eachChild(function(currentNode){
				if ( currentNode !== undefined ) {
		          	for (var j = 0, lenValues = baseParams.arrayOfValues.length; j < lenValues; j++) {
		          		if ( currentNode.attributes.text == baseParams.arrayOfValues[j] ) {
		          			currentNode.getUI().toggleCheck(true);
		          		}
		          	}
				}
			});
		}
    }
  
	  ,onSelectionChange:function(object) {
	      // do whatever is necessary to assign the employee to position
		// notify Observers directly
		  this.notifyObservers('selectionchange', {
			  widget: 'range', 
			  rootNode: this.getRootNode(), 
			  selectedValues: baseParams.arrayOfValues}
		  );
	  }
    
  });

  Neologism.rangeTermsTree.objectToRender = objectToRender;
  
  /**
   * this function is used to clear the arrayOfValues and notify the observe 
   */
  Neologism.rangeTermsTree.clearValues = function() {
	  //console.log('clearValues called');
	  this.arrayOfValues.length = 0;
	  this.refresh();
	  //this.fireEvent('selectionchange', null);
  };
  
};