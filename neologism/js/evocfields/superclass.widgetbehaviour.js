/**
 * @author guicec
 */

/**
 * Create the class selection widget behaviour for filed_superclass2 
 * 
 * @param {Object} field_name
 */
Neologism.createSuperclassSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];

  Neologism.superclassesTreePanel = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Classes'),
    disabled: false,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,
      listeners: {
        // load : ( Object This, Object node, Object response )
        // Fires when the node has been successfuly loaded.
        // added event to refresh the checkbox from its parent 
        load: function(loader, node, response){
          	
    		node.eachChild(function(currentNode){
    			currentNode.cascade( function() {
    				this.expand();
    				
	            	if (this.text == editingValue) {
	            		this.remove();
		            }
	              
	              	for (var j = 0, lenValues = baseParams.arrayOfValues.length; j < lenValues; j++) {
	              		if ( this.attributes.text == baseParams.arrayOfValues[j] ) {
	              			this.getUI().toggleCheck(true);
	              		}
	              	}
	            }, null);
	         });
    		
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
	            
        		// check if this node has more than 1 super class, so we need to checked it 
        		// in other places in the tree.
        		if( node.attributes.superclasses !== undefined ) {
        			var c = node.attributes.superclasses;
	        		var len = c.length;
	        		if ( len > 1 ) {
	        			var tree = node.getOwnerTree(); 
		        		var rootnode = tree.getRootNode(); 
		        		for ( var i = 0; i < len; i++ ) {
		        			if( c[i] != node.parentNode.text ) {
		        				var currentNode = tree.findNodeByText(c[i]);
		        				if ( currentNode !== null ) {
		        					currentNode.expand();
		        					var n = currentNode.findChild('text', node.text);
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
		                //var cid = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
	            		if (node.text != this.text && node.attributes.nodeStatus != Ext.tree.TreePanel.nodeStatus.BLOCKED
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
	    			if ( baseParams.arrayOfValues[i] == node.text ) {
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
	        			var tree = node.getOwnerTree(); 
		        		var rootnode = tree.getRootNode(); 
		        		for ( var i = 0; i < len; i++ ) {
		        			if( c[i] != node.parentNode.text ) {
		        				var currentNode = tree.findNodeByText(c[i]);
		        				if ( currentNode !== null ) {
		        					currentNode.expand();
		        					var n = currentNode.findChild('text', node.text);
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
	        	
	        	//var cid = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
        		if ( node.text != this.text  ) {
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
    
  	// override method onSelectionChange called when a fireEvent('selectionchange', ...); is invoked
  	,onSelectionChange:function(object) {
        // do whatever is necessary to assign the employee to position
    	// notify Observers if there is someone
    	this.notifyObservers('selectionchange', object);
    },
    
    updatselection: function(){
      this.root.eachChild(function(currentNode){
        currentNode.cascade(function(){
          // expand the node to iterate over it
          this.getOwnerTree().expandPath(this.getPath());
          
          if (this.id == editingValue) {
            this.getUI().addClass('locked-for-edition');
            this.getUI().checkbox.disabled = true;
            this.getUI().checkbox.checked = false;
          }
          
          for (var j = 0, lenValues = baseParams.arrayOfValues.length; j < lenValues; j++) {
            if (this.id == baseParams.arrayOfValues[j]) {
              this.getUI().toggleCheck(true);
            }
          }
        }, null);
      });
    }
    
  });

  Neologism.superclassesTreePanel.objectToRender = objectToRender;
};
