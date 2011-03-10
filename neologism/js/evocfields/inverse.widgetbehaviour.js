/**
 * Create the inverse selection widget behaviour for filed_inverse2
 * 
 * @param {Object} field_name
 */
Neologism.createInverseSelecctionWidget = function( field_name ) {
  
	var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
	var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
	var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
	
	// we need to past the baseParams as and object, that is why we creat the baseParams object
	// and add the arrayOfValues array 
	var baseParams = {};
	Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
	baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];
	
//	var domain = [];
//	var range = [];
//	var lastSender = null;
	
	var parentPaths = new Array();
	var pathsToExpand = new Array();
	var treeloaded = false;
	 
	Neologism.inverseTermsTree = new Neologism.TermsTree({
	    //renderTo: objectToRender,
	    title: Drupal.t('Classes'),
	    disabled: false,
	    height: 200,
	    
	    loader: new Ext.tree.TreeLoader({
	      dataUrl: dataUrl,
	      baseParams: baseParams,
	      
	      listeners: {
	        // load : ( Object This, Object node, Object response )
	        // Fires when the node has been successfuly loaded.
	        // added event to refresh the checkbox from its parent 
	        load: function(loader, node, response){
		    	var treePanel = node.getOwnerTree();
				for (var i = 0; i < pathsToExpand.length; i++) {
					treePanel.expandPath(pathsToExpand[i]);
				}
	        }
	      }
	    }),
	    
	    // SET the root node.
	    root: new Ext.tree.AsyncTreeNode({
	      text	: Drupal.t('Thing / Superclass'),
	      id		: 'root',                  // this IS the id of the startnode
	      iconCls: 'class-samevoc',
	      disabled: true,
	      expanded: false,
	      
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
			// behaviour for on checkchange in superclassesTree TreePanel object 
			checkchange: function(node, checked) {
				// check for node references that should be updated together
		  		node.checkNodeReferences(checked);
  		
				if ( checked ) {
					// if we're checking the box, check it all the way up
					if( !Neologism.util.in_array(node.attributes.text, baseParams.arrayOfValues)) {
						baseParams.arrayOfValues.push(node.attributes.text);
					}
				} else {
					Neologism.util.remove_element(node.attributes.text, baseParams.arrayOfValues);
			    }
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
						if (treeloaded) {
							this.checkInverses(currentNode, editingValue);
						}
					}
				});
				// if the editting node was found then it must be removed
				if (node_to_remove != null) node_to_remove.remove();
			}
		} // listeners
		/*
		,onSelectionChange:function(objectSender) {
			// do whatever is necessary to assign the employee to position
		  	if( objectSender != null ) {
		  		lastSender = objectSender;
			  	if( objectSender.widget == 'domain' ) {
			  		domain = objectSender.selectedValues;
			  	}
			  	if( objectSender.widget == 'range' ) {
			  		range = objectSender.selectedValues;
			  	}
			  	
			  	var allowedAsInverseProperties = this.computeInverses(lastSender.rootNode, 
			  			((domain.length == 0) ? ['rdfs:Resource'] : domain), 
			  			((range.length == 0) ? ['rdfs:Resource'] : range)
			  		);
			  	this.getRootNode().eachChild(function(currentNode) {
			  		// we need to expand the node to traverse it
			  		currentNode.cascade(function() {
			        	// we need to expand the node to traverse it
			        	this.expand();
			        	
			        	if ( allowedAsInverseProperties.indexOf(this.text) == -1 ) {
			        		this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED;
							this.getUI().addClass('class-bloked');
						}
			        	else {
			        		this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
							this.getUI().removeClass('class-bloked');
			        	}
			        	
			        	this.getUI().checkbox.disabled = (this.attributes.nodeStatus != Ext.tree.TreePanel.nodeStatus.NORMAL);
			        	
			        });
			    });
			  	
			  	this.enable();
		  	}
		}
		*/
	  });
	
	Neologism.inverseTermsTree.objectToRender = objectToRender;
};
