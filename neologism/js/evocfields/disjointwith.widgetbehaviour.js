/**
 * widget behaviour for field_disjointwith2 field
 * 
 * @param {Object} field_name
 */
Neologism.createDisjointwithSelecctionWidget = function(field_name) {
  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];
  
  Neologism.disjointwithTreePanel = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Disjoint with class(es)'),
    disabled: false,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,//baseParams,
      listeners: {
    	load: function(loader, node, response){
    		var treePanel = node.getOwnerTree();
			Neologism.TermsTree.traverse(node, function(currentNode, path) {
				if( Neologism.util.in_array(currentNode.text, baseParams.arrayOfValues) ) {
					path.pop();
					treePanel.expandPath(path.join('/'));
				}
			}, true);
        }
      }
    }),
    
    // SET the root node.
    root: new Ext.tree.AsyncTreeNode({
      text: Drupal.t('Thing / Superclass'),
      id: 'root', // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false
    }),
    
    listeners: {
	  	// behaviour for on checkchange in Neologism.superclassesTree TreePanel object 
    	checkchange: function(node, checked) {
	  		node.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
	  		
	        if ( checked  ) {
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
	    		
	        } // else
	        
	        //this.fireEvent('selectionchange', node);
		} // checkchange 
  
		  ,expandnode: function( node ) {
			  var node_to_remove = null;
				node.eachChild(function(currentNode){
					if ( currentNode !== undefined ) {
						if (currentNode.attributes.text == editingValue) {
							node_to_remove = currentNode;
			            }
						else if( Neologism.util.in_array(currentNode.attributes.text, baseParams.arrayOfValues)) {
							currentNode.getUI().toggleCheck(true);
						}
						
					}
				});
				// if the editting node was found then it must be removed
				if (node_to_remove != null) node_to_remove.remove();
			}
      
    } // listeners
  
  	//this event sometime is fired for other component
   
  	,onSelectionChange:function(node) {
	      // do whatever is necessary to assign the employee to position
  	}
    
  });
  
  Neologism.disjointwithTreePanel.objectToRender = objectToRender;
  	
};
