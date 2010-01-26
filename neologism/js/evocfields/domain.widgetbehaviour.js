/**
 * @author guicec
 */

/**
 * Create the class selection widget behaviour for filed_superclass2 
 * 
 * @param {Object} field_name
 */
Neologism.createDomainSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.neologism.field_id[field_name];
  var dataUrl = Drupal.settings.neologism.json_url[field_name];
  var editingValue = Drupal.settings.neologism.editing_value[field_name];
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.neologism.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.neologism.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.neologism.field_values[field_name];

  Neologism.domainsTermsTree = new Neologism.TermsTree({
    renderTo: objectToRender,
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
          	
    		console.log(node);
          	
    		var editingNode = null;
    		
    		node.eachChild(function(currentNode){
	            currentNode.cascade( function() {
	            	// expand the node to iterate over it
	            	var id = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
	            	this.getOwnerTree().expandPath(this.getPath());
	              
	              	if ( id == editingValue ) {
						this.getUI().addClass('locked-for-edition');
						this.getUI().checkbox.disabled = true;
						this.getUI().checkbox.checked = false;
						editingNode = this;
						//this.remove();
	              	}
	              
	              	for (var j = 0, lenValues = baseParams.arrayOfValues.length; j < lenValues; j++) {
	              		if ( id == baseParams.arrayOfValues[j] ) {
	              			this.getUI().toggleCheck(true);
	              		}
	              	}
	            }, null);
	         });
    		
    		// we remove the editing node from treeview for edition 
    		// TODO at this point we also need to configure the treeview because could be possible that the editing
    		// class be disjoint with some future superclass
    		if( editingNode != null ) {
    			 editingNode.remove();
    		}
	          
    		// disjoiness depend from classes selection
    		//Neologism.disjointWithTreePanel.render(Neologism.objectToRender);
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
	  		//console.info(node);
	  		var id = ( node.attributes.realid !== undefined ) ? node.attributes.realid : node.id;
	  		
	        if ( checked /*&& node.parentNode !== null*/ ) {
		        // add selection to array of values
        		if ( baseParams.arrayOfValues.indexOf(id) == -1 ) {
	            	baseParams.arrayOfValues.push(id);
	            }
	            
        		console.log(baseParams.arrayOfValues);
        		
        		// check if this node has more than 1 super class, so we need to checked it 
        		// in other places in the tree.
        		if( node.attributes.superclasses !== undefined ) {
	        		var c = node.attributes.superclasses;
	        		var len = c.length;
	        		if ( len > 1 ) {
		        		var rootnode = node.getOwnerTree().getRootNode(); 
		        		for ( var i = 0; i < len; i++ ) {
		        			if( c[i] != node.parentNode.id ) {
		        				var currentNode = node.getOwnerTree().findNodeById(c[i]);
		        				if ( currentNode !== null ) {
		        					var n = currentNode.findChild('text', id);
		        					if( n.getUI().checkbox.checked == false ) {
		        						n.getUI().toggleCheck(true);
		        					}
		        				}
		        			}
		        		}
	        		}
        		}
        		
	            // disabled all the parent of the selection
        		if ( !node.parentNode.isRoot ) {
	            	node.bubble( function() {
		                var cid = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
	            		if (id != cid && node.attributes.nodeStatus != Ext.tree.TreePanel.nodeStatus.BLOCKED
	            				&& node.getUI().nodeClass != 'locked-for-edition') {
		                	this.getUI().checkbox.disabled = true;
		                	this.getUI().checkbox.checked = true;
		                }
		                // if this node is the root node then return false to stop the bubble process
		                if ( this.parentNode.isRoot ) {
		                	return false;
		                }
	            	});
	            }
	    	} 
	        else {
	    		// if we are unchecked a checkbox
	    		for ( var i = 0, len = baseParams.arrayOfValues.length; i < len; i++ ) {
	    			if ( baseParams.arrayOfValues[i] == id ) {
	    				baseParams.arrayOfValues.splice(i, 1);
	    			}
	    		}
	    		
	    		// check if we can enabled a parent after a deselection
				if (!node.parentNode.isRoot) {
				    // search for someone checked
					var someoneChecked = false;
					node.bubble( function() {
				    	if ( id != this.id ) {
				    		if ( this.getOwnerTree().isSomeChildChecked(this) ) {
				    			return false;
				    		}
				    		this.getUI().checkbox.disabled = false;
				    		this.getUI().checkbox.checked = false;
				    	}
				    	
				    	if ( this.parentNode.isRoot ) {
				    		return false;
				    	}
				    });
					
				}
				
				// check multiple superclasses dependencies
				if( node.attributes.superclasses !== undefined ) {
	        		var c = node.attributes.superclasses;
	        		var len = c.length;
	        		if ( len > 1 ) {
		        		var rootnode = node.getOwnerTree().getRootNode(); 
		        		for ( var i = 0; i < len; i++ ) {
		        			if( c[i] != node.parentNode.id ) {
		        				var currentNode = node.getOwnerTree().findNodeById(c[i]);
		        				//console.info(currentNode);
		        				//console.log('Path: %s, c[i]: %s', rootnode.findChild('id', c[i]), c[i]); //.getPath());
		        				if ( currentNode !== null ) {
		        					var n = currentNode.findChild('text', id);
		        					if( n.getUI().checkbox.checked == true ) {
		        						n.getUI().toggleCheck(false);
		        					}
		        				}
		        			}
		        		}
	        		}
        		}
	        } // else
	        
	        // check for disjointwith classes with the selected class
    		node.getOwnerTree().checkDisjointWith(node);
    		
	        // disabled or enabled child in cascade depending of the value of checked
	        node.cascade( function() {
	        	if( !this.isExpanded() ) {
	        		this.expand();
	        	}
	        	
	        	var cid = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
        		if ( id != cid  ) {
        			if ( checked ) {
        				if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.INCONSISTENT ) {
        					this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT;
        				}
        				else if ( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.NORMAL ) {
        					this.getUI().addClass('class-bloked');
	        				this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED;
        				}
        			}
        			else {
        				if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT ) {
        					this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.INCONSISTENT;
        				}
        				else if ( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED ) {
	        				this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
	        				this.getUI().removeClass('class-bloked');
        				}
        			}
        			
        			this.getUI().checkbox.disabled = !this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.NORMAL;
        		}
	        });
	        
	        this.fireEvent('selectionchange', node);
	        
	        
  		} // checkchange  
    }
    
  ,onSelectionChange:function(object) {
      // do whatever is necessary to assign the employee to position
	// notify Observers directly
  	console.log('domain on select');
	  this.notifyObservers('selectionchange', {
		  widget: 'domain', 
		  rootNode: this.getRootNode(), 
		  selectedValues: baseParams.arrayOfValues}
	  );
  }
    
  });

};