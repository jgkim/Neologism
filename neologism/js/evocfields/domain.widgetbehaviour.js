/**
 * @author guicec
 */

/**
 * Create the class selection widget behaviour for filed_superclass2 
 * 
 * @param {Object} field_name
 */
Neologism.createDomainSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];

  Neologism.domainTermsTree = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Domain'),
    disabled: false,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,
      listeners: {
        // load : ( Object This, Object node, Object response )
        // Fires when the node has been successfuly loaded.
        // added event to refresh the checkbox from its parent 
        load: function(loader, node, response){
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
	  		var id = ( node.attributes.realid !== undefined ) ? node.attributes.realid : node.id;
	  		
	        if ( checked /*&& node.parentNode !== null*/ ) {
		        // add selection to array of values
        		if ( baseParams.arrayOfValues.indexOf(id) == -1 ) {
	            	baseParams.arrayOfValues.push(id);
	            }
	    	} 
	        else {
	    		// if we are unchecked a checkbox
	    		for ( var i = 0, len = baseParams.arrayOfValues.length; i < len; i++ ) {
	    			if ( baseParams.arrayOfValues[i] == id ) {
	    				baseParams.arrayOfValues.splice(i, 1);
	    			}
	    		}
	        } // else
	        
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
  
  		,selectionchange: function(objectSender) {
  		}
    }
    
  ,onSelectionChange:function(object) {
      // do whatever is necessary to assign the employee to position
	// notify Observers directly
	  this.notifyObservers('selectionchange', {
		  widget: 'domain', 
		  rootNode: this.getRootNode(), 
		  selectedValues: baseParams.arrayOfValues}
	  );
  }
    
  });

  Neologism.domainTermsTree.objectToRender = objectToRender;
};