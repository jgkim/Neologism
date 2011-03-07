/**
 * widget behaviour for field_disjointwith2 field
 * 
 * @param {Object} field_name
 */

Neologism.DisjointnessTermsTree = Ext.extend(Neologism.TermsTree, {
});

Neologism.createDisjointwithSelecctionWidget = function(field_name) {
  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  var parentPaths = new Array();
  var pathsToExpand = new Array();
  
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];
  var treeloaded = false;
  
  Neologism.disjointwithTreePanel = new Neologism.DisjointnessTermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Disjoint with class(es)'),
    disabled: false,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,//baseParams,
      preloadChildren: true,
      listeners: {
    	load: function(loader, node, response){
    		var treePanel = node.getOwnerTree();
			for (var i = 0; i < pathsToExpand.length; i++) {
				treePanel.expandPath(pathsToExpand[i]);
			}
			
			//TODO: after got all the paths disable all the parent of editing value
			// because they cannot be possible disjointness. This widget should be synchronized
			// with superclasses widget.
        }
      }
    }),
    
    // SET the root node.
    root: new Ext.tree.AsyncTreeNode({
      text: Drupal.t('Thing / Superclass'),
      id: 'root', // this IS the id of the startnode
      iconCls: 'class-samevoc',
    	  
      listeners: {
    	beforeexpand: function( /*Node*/ node, /*Boolean*/ deep, /*Boolean*/ anim ) {
	    	if (treeloaded) return;
	    	
    		var treePanel = node.getOwnerTree();
			parentPaths = [];
			Neologism.TermsTree.traverse(node, function(currentNode, path) {
				if( Neologism.util.in_array(currentNode.text, baseParams.arrayOfValues) ) {
					var pathToExpand = path.slice();
					pathToExpand.pop();
					pathsToExpand.push(pathToExpand.join('/'));
				}
				
				// get all paths of the editing value
				if (currentNode.text == editingValue) {
					var pathCopy = path.slice();
					pathCopy.pop();
					parentPaths.push(pathCopy.slice());
				}
			}, true);
			
			treeloaded = true;
    	}
    }
    }),
    
    listeners: {
	  	// behaviour for on checkchange in Neologism.superclassesTree TreePanel object 
    	checkchange: function(node, checked) {
	  		//node.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
	  		// the call comes from checkDisjointness method the event must be cancelled.
	  		if (checked && Neologism.util.in_array(editingValue, node.attributes.disjointwith)) 
	  			return;
	  		
	  		// check for node references that should be updated together
	  		node.checkNodeReferences(checked);
	  		
	        if ( checked  ) {
		        // add selection to array of values
	        	if( !Neologism.util.in_array(node.attributes.text, baseParams.arrayOfValues)) {
					baseParams.arrayOfValues.push(node.attributes.text);
				}
		    } 
	        else {
	    		// if we are unchecked a checkbox
	        	Neologism.util.remove_element(node.attributes.text, baseParams.arrayOfValues);
	        } // else
	        
	        //this.fireEvent('selectionchange', node);
		} // checkchange 
  
		,expandnode: function( node ) {
			var node_to_remove = null;
			node.eachChild(function(currentNode){
				if ( currentNode !== undefined ) {
					if (currentNode.attributes.text == editingValue) {
						//currentNode.disable();
						node_to_remove = currentNode;
			        }
					else if( Neologism.util.in_array(currentNode.attributes.text, baseParams.arrayOfValues)) {
						currentNode.getUI().toggleCheck(true);
					}
					
					if (treeloaded) {
						this.checkDisjointness(currentNode, editingValue, parentPaths);
					}
				}
			});
			// if the editting node was found then it must be removed
			if (node_to_remove != null) node_to_remove.remove();
		}
      
    } // listeners
  
  	//this event sometime is fired for other component
  	,onSelectionChange:function(node) {
  	}
    
  });
  
  Neologism.disjointwithTreePanel.objectToRender = objectToRender;
};
