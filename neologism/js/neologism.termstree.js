/**
 * @author guicec
 */

//Ext.ns('Neologism');

/**
 * Override TreePanel onClick and onDblClick events
 * @param {Object} e
 */ 
Ext.override(Ext.tree.TreeNodeUI, {
  onClick : function(e) { //debugger;
    if ( this.dropping ) {
      e.stopEvent();
      return;
    }
    
    if ( this.fireEvent("beforeclick", this.node, e) !== false ) {
      var a = e.getTarget('a');
      if ( !this.disabled && this.node.attributes.href && a ){
        this.fireEvent("click", this.node, e);
        return;
      }	else { 
    	  if ( a && e.ctrlKey ) {
    		  e.stopEvent();
    	  }
      }
      
      e.preventDefault();
      if (this.disabled) {
        return;
      }
      if ( this.node.attributes.singleClickExpand && !this.animating && this.node.hasChildNodes() ) {
        //this.node.expand(); 
        //this.node.toggle();
      }
  
      this.fireEvent("click", this.node, e);
      //alert('onclick');
    } else {
      e.stopEvent();
    }
    
  }
});

Ext.override(Ext.tree.TreeNodeUI, {
  onDblClick : function(e){ //debugger;
    e.preventDefault();
    if ( this.disabled ){
      return;
    }
    if ( this.checkbox ){
      return;
      // cancel the toggleCheck when dblclick
      //this.toggleCheck();
    }
    if ( this.animating && this.node.hasChildNodes() ){
      //this.node.toggle();
      //this.node.expand();
    }
    this.fireEvent("dblclick", this.node, e);
  }
});

/*
Ext.override(Ext.tree.TreeNodeUI, {
    toggleCheck : function(value) {
	alert('toggleCheck');
		var cb = this.checkbox;
        if(cb){
            var checkvalue = (value === undefined ? !cb.checked : value);
            cb.checked = checkvalue;
            this.node.attributes.checked = checkvalue;
        }
    }
}); 
*/


/* Extending/depending on:  
~ = modified function (when updating from SVN be sure to check these for changes, especially to Ext.tree.TreeNodeUI.render() )  
+ = added function  
  
TreeSelectionModel.js  
    Ext.tree.CheckNodeMultiSelectionModel : ~init(), ~onNodeClick(), +extendSelection(), ~onKeyDown()  
  
TreeNodeUI.js  
    Ext.tree.CheckboxNodeUI : ~render(), +checked(), +check(), +toggleCheck()  
  
TreePanel.js  
    Ext.tree.TreePanel : +getChecked()  
  
TreeLoader.js  
    Ext.tree.TreeLoader : ~createNode()  
  
*/   
   
/**  
 * Retrieve an array of ids of checked nodes  
 * @return {Array} array of ids of checked nodes  
 */   
Ext.tree.TreePanel.prototype.getChecked = function(node){   
    var checked = [], i;   
    if( typeof node == 'undefined' ) {   
        node = this.rootVisible ? this.getRootNode() : this.getRootNode().firstChild;   
    }   
   
    if( node.attributes.checked ) {   
        checked.push(node.id);   
        if( !node.isLeaf() ) {   
            for( i = 0; i < node.childNodes.length; i++ ) {   
                checked = checked.concat( this.getChecked(node.childNodes[i]) );   
            }   
        }   
    }   
    return checked;   
};  

//-------------------------------------------------------
// especifies methods for TermsTree



/**
 * Note: we are working with node.text becuase the id could be modified or had not the right value
 */
Ext.tree.TreePanel.prototype.someChildOrParentIsChecked = function(node, fn) { 
	var someoneChecked = false;
	
	if (!node.parentNode.isRoot) {
	    // search for someone checked
		var someoneChecked = false;
		node.bubble( function() {
	    	if ( node.text != this.text ) {
	    		this.eachChild( function(currentNode) {
	    			currentNode.cascade( function() {
	    				//var cid = ( this.attributes.realid !== undefined ) ? this.attributes.realid : this.id;
		    			if ( this.getUI().checkbox.checked == true ) {
		    				someoneChecked = true;
	                		return false;
	                	}
		            }, null);
		        });
	    		
	    		/*
	    		if ( !someoneChecked ) {
	    			if(fn.call() === false){
	                //    break;
	                }
	    			//this.getUI().checkbox.disabled = false;
	    		}
	    		*/
	    	}
	    	
	    	if ( this.parentNode.isRoot ) {
	    		return false;
	    	}
	    });
	}
};

Ext.tree.TreePanel.nodeStatus = {
		NORMAL: 0,
		IMPLICIT: 1,
		INCONSISTENT: 2,
		BLOCKED: 3,
		BLOCKED_AND_INCONSISTENT: 4
};

Ext.tree.TreePanel.prototype.checkDisjointWith = function(node){   
	var disjointwith = [];
	var checked = node.getUI().checkbox.checked;
	
	if( node.attributes.disjointwith.length > 0 ) {
		disjointwith = disjointwith.concat(node.attributes.disjointwith);
	}
	
	// traverse children
	/*
	node.eachChild( function(currentNode) {
		currentNode.cascade(function() {
        	console.log(this.text);
        	console.log(this.ui);
        	console.log(this.getUI().nodeClass);
			if( this.getUI().nodeClass != 'class-disabled' && this.attributes.disjointwith.length > 0 ) {
				disjointwith = disjointwith.concat(this.attributes.disjointwith);
			}
        }, null);
    });
	*/
	//console.log(disjointwith);
	
	var len = disjointwith.length;
	if( len > 0 ) {
		// get the root node
		var rootnode = node.getOwnerTree().getRootNode();
		// traverse the tree 
		rootnode.eachChild( function(currentNode) {
			currentNode.cascade( function() {
				for ( var i = 0; i < len; i++ ) {
					if ( this.text == disjointwith[i] ) {
						if( checked ) {
							if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED ) {
								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT;
								this.getUI().removeClass('class-bloked');
							} else {
								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.INCONSISTENT;
							}
							this.getUI().addClass('class-inconsistent');
			            }
			            else {
			            	if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT ) {
								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED;
								this.getUI().addClass('class-bloked');
							} else {
								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
							}
			            	this.getUI().removeClass('class-inconsistent');
			            	
			            }
						this.getUI().checkbox.disabled = (this.attributes.nodeStatus != Ext.tree.TreePanel.nodeStatus.NORMAL);
						
						// all child also are disjoint
						this.eachChild( function(currentNode) {
	        	            currentNode.cascade( function() {
	        	            	if( checked ) {
	    							if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED ) {
	    								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT;
	    								this.getUI().removeClass('class-bloked');
	    							} else {
	    								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.INCONSISTENT;
	    							}
	    							this.getUI().addClass('class-inconsistent');
	    			            }
	    			            else {
	    			            	if( this.attributes.nodeStatus == Ext.tree.TreePanel.nodeStatus.BLOCKED_AND_INCONSISTENT ) {
	    								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.BLOCKED;
	    								this.getUI().addClass('class-bloked');
	    							} else {
	    								this.attributes.nodeStatus = Ext.tree.TreePanel.nodeStatus.NORMAL;
	    							}
	    			            	this.getUI().removeClass('class-inconsistent');
	    			            }
	    						this.getUI().checkbox.disabled = (this.attributes.nodeStatus != Ext.tree.TreePanel.nodeStatus.NORMAL);
	        	            }, null);
	        	        });
					}
				}
            }, null);
        });
	}
};

Ext.tree.TreePanel.prototype.isSomeChildChecked = function(node){  
	var someoneChecked = false;
	node.eachChild( function(currentNode) {
		//currentNode.cascade( function() {
			var cid = ( currentNode.attributes.realid !== undefined ) ? currentNode.attributes.realid : currentNode.id;
			if ( currentNode.getUI().checkbox.checked == true ) {
				someoneChecked = true;
	    		return false;
	    	}
	   // }, null);
	});
	
	return someoneChecked;
};

Ext.tree.TreePanel.prototype.isSomeChildCheckedOrStatus = function(node, status){  
	var someoneChecked = false;
	node.eachChild( function(currentNode) {
		//currentNode.cascade( function() {
			var cid = ( currentNode.attributes.realid !== undefined ) ? currentNode.attributes.realid : currentNode.id;
			if ( currentNode.getUI().checkbox.checked == true || currentNode.attributes.nodeStatus == status ) {
				someoneChecked = true;
	    		return false;
	    	}
			
	   // }, null);
	});
	
	return someoneChecked;
}

;
 
//Ext.extend(Ext.tree.CheckboxNodeUI, Ext.tree.TreeNodeUI, {   
    /**  
     * This is virtually identical to Ext.tree.TreeNodeUI.render, modifications are indicated inline  
     */   
/*
	render : function(bulkRender){   
        var n = this.node;   
        var targetNode = n.parentNode ?   
            n.parentNode.ui.getContainer() : n.ownerTree.container.dom; // in later svn builds this changes to n.ownerTree.innerCt.dom   
        if(!this.rendered){   
            this.rendered = true;   
            var a = n.attributes;   
   
            // add some indent caching, this helps performance when rendering a large tree   
            this.indentMarkup = "";   
            if(n.parentNode){   
                this.indentMarkup = n.parentNode.ui.getChildIndent();   
            }   
   
            // modification: added checkbox   
            var buf = ['<li class="x-tree-node"><div class="x-tree-node-el ', n.attributes.cls,'">',   
                '<span class="x-tree-node-indent">',this.indentMarkup,"</span>",   
                '<img src="', this.emptyIcon, '" class="x-tree-ec-icon">',   
                '<img src="', a.icon || this.emptyIcon, '" class="x-tree-node-icon',(a.icon ? " x-tree-node-inline-icon"="" :="" ),(a.iconcls="" ?="" +a.iconcls="" ),'"="" unselectable="on">',   
                '<input class="l-tcb" type="checkbox" ,="" (a.checked="" ?="" checked="">" : ">")'
                '<a hidefocus="on" href="',a.href ? a.href : " #",'"="" ,="" a.hreftarget="" ?="" target="'+a.hrefTarget+'" :="">',   
                 '<span unselectable="on">',n.text,"</span></a></div>",   
                '<ul class="x-tree-node-ct" style="display:none;"></ul>',   
                "</li>"];   
   
            if(bulkRender !== true && n.nextSibling && n.nextSibling.ui.getEl()){   
                this.wrap = Ext.DomHelper.insertHtml("beforeBegin",   
                                                            n.nextSibling.ui.getEl(), buf.join(""));   
            }else{   
                this.wrap = Ext.DomHelper.insertHtml("beforeEnd", targetNode, buf.join(""));   
            }   
            this.elNode = this.wrap.childNodes[0];   
            this.ctNode = this.wrap.childNodes[1];   
            var cs = this.elNode.childNodes;   
            this.indentNode = cs[0];   
            this.ecNode = cs[1];   
            this.iconNode = cs[2];   
            this.checkbox = cs[3]; // modification: inserted checkbox   
            this.anchor = cs[4];   
            this.textNode = cs[4].firstChild;   
            if(a.qtip){   
             if(this.textNode.setAttributeNS){   
                 this.textNode.setAttributeNS("ext", "qtip", a.qtip);   
                 if(a.qtipTitle){   
                     this.textNode.setAttributeNS("ext", "qtitle", a.qtipTitle);   
                 }   
             }else{   
                 this.textNode.setAttribute("ext:qtip", a.qtip);   
                 if(a.qtipTitle){   
                     this.textNode.setAttribute("ext:qtitle", a.qtipTitle);   
                 }   
             }   
            } else if(a.qtipCfg) {   
                a.qtipCfg.target = Ext.id(this.textNode);   
                Ext.QuickTips.register(a.qtipCfg);   
            }   
   
            this.initEvents();   
   
            // modification: Add additional handlers here to avoid modifying Ext.tree.TreeNodeUI   
            Ext.fly(this.checkbox).on('click', this.check.createDelegate(this, [null]));   
            n.on('dblclick', function(e) {   
                if( this.isLeaf() ) {   
                    this.getUI().toggleCheck();   
                }   
            });   
   
            if(!this.node.expanded){   
                this.updateExpandIcon();   
            }   
        }else{   
            if(bulkRender === true) {   
                targetNode.appendChild(this.wrap);   
            }   
        }   
    },   
   
    checked : function() {   
        return this.checkbox.checked;   
    },   
   
    /**  
     * Sets a checkbox appropriately.  By default only walks down through child nodes  
     * if called with no arguments (onchange event from the checkbox), otherwise  
     * it's assumed the call is being made programatically and the correct arguments are provided.  
     * @param {Boolean} state true to check the checkbox, false to clear it. (defaults to the opposite of the checkbox.checked)  
     * @param {Boolean} descend true to walk through the nodes children and set their checkbox values. (defaults to false)  
     */  
    
    /*
    check : function(state, descend, bulk) {   
        var n = this.node;   
        var tree = n.getOwnerTree();   
        var parentNode = n.parentNode;n   
        if( !n.expanded && !n.childrenRendered ) {   
            n.expand(false, false, this.check.createDelegate(this, arguments));   
        }   
   
        if( typeof bulk == 'undefined' ) {   
            bulk = false;   
        }   
        if( typeof state == 'undefined' || state === null ) {   
            state = this.checkbox.checked;   
            descend = !state;   
            if( state ) {   
                n.expand(false, false);   
            }   
        } else {   
            this.checkbox.checked = state;   
        }   
        n.attributes.checked = state;   
   
        // do we have parents?   
        if( parentNode !== null && state ) {   
            // if we're checking the box, check it all the way up   
            if( parentNode.getUI().check ) {   
                parentNode.getUI().check(state, false, true);   
            }   
        }   
        if( descend && !n.isLeaf() ) {   
            var cs = n.childNodes;   
      for(var i = 0; i < cs.length; i++) {   
        cs[i].getUI().check(state, true, true);   
      }   
        }   
        if( !bulk ) {   
            tree.fireEvent('check', n, state);   
        }   
    },   
   
    toggleCheck : function(state) {   
        this.check(!this.checkbox.checked, true);   
    }   
   
});   
*/

Neologism.TermsTree = Ext.extend(Ext.tree.TreePanel, {
  
  //props (overridable by caller)
  height           : 400,
  width            : '100%',
  disabled         : false,
  rootVisible      : false,
  header           : false,
  headerAsText     : false, // hidden the header title

  initComponent: function() {
    // Called during component initialization
    var config = {
      //props (non-overridable)
      
      //------------------------------------------- standard TreePanel properties
      useArrows        : true,  
      collapsible      : true,
      animCollapse     : true,
      border           : true,
      autoScroll       : true,
      animate          : true,
      containerScroll  : true,
      enableDD         : false,
      singleClickExpand: true,

      tbar: [
        {
          tooltip: 'Refresh the tree',
          iconCls: 'x-tbar-loading',
          scope: this,
          handler: function(){ 
            this.refresh(); 
          }
        },
        {
          tooltip: 'Expand all',
          iconCls: 'icon-expand-all',
          scope: this,
          handler: function(){ 
            this.expandAll(); 
          }
        },
        {
          tooltip: 'Collapse all',
          iconCls: 'icon-collapse-all',
          scope: this,
          handler: function(){ 
            this.collapseAll(); 
          }
        },
        '-'
      ],
      
      //------------------------------------------- 
      // standard TreePanel properties

      //-------------------------------------------
      // custom TermsTree properties 
      hiddenNodes: [],
      // array of selected values
      arrayOfValues: []
    };
  
    // public property
    this.observers = [];
    
    
    // Config object has already been applied to 'this' so properties can 
    // be overriden here or new properties (e.g. items, tools, buttons) 
    // can be added, eg:
    Ext.apply(this, config);
    Ext.apply(this.initialConfig, config);

    // Call parent (required)
    Neologism.TermsTree.superclass.initComponent.apply(this, arguments);

    // After parent code
    // e.g. install event handlers on rendered component
    // event definition for
    //  - node click: so we can refresh the list of notes
    //  - notes drop: so we can re-assign notes to a new neod
    this.addEvents('nodeclick', 'notesdrop');
    
    // we also want that tree terms send a message when selection has been changed
    this.addEvents('selectionchange');
    this.addListener({
    	selectionchange:{ fn: this.onSelectionChange, scope: this } 
    });
    //this.enableBubble('selectionchange');
    
    // Filter text field that will be added to the Tool bar and perform the
    // filtering in the tree of nodes
    this.filterField = new Ext.form.TextField({
      width: 300,
      emptyText: Drupal.t('Type term to search'),
      tree: this,
      listeners:{
        render: function(f){
            f.el.on('keydown', function(ev) { 
              this.tree.filterNodes( this.getValue() );
            }, this, {buffer: 350});
        }
      }
    });
    
    // add the text field to the toolbar
    this.getTopToolbar().push( this.filterField );

    //------------------------------------------- 
    //  event handlers

    // Handle node click  
    this.on('click', function(node) {
      this.fireEvent('nodeclick', node.id);
    });
  },

  // other methods/actions
  filterNodes: function(pattern){
    // un-hide the nodes that were filtered last time
    Ext.each(this.hiddenNodes, function(n){
      n.ui.removeClass('match-search');
      n.ui.show();
 		});

    if(!pattern){
 		return;
 	}
    
    this.expandAll();
		
    var re = new RegExp('^.*' + Ext.escapeRe(pattern) + '.*', 'i');

    this.root.cascade( function(n){
      if (re.test(n.text)) {
        n.ui.addClass('match-search');
        n.ui.show();
        n.bubble( 
          function() { 
            this.ui.show(); 
          }
        );
      } else {
        n.ui.hide();
        this.hiddenNodes.push(n);
      }
    }, this);
  }

  ,refresh:function(){
    this.loader.load( this.getRootNode() );
  }
  
  ,onSelectionChange:function(newValues) {
      // do whatever is necessary to assign the employee to position
	  //console.log('TermsTree - onSelectionChange not implemented yet...');
  }
  
});

//this to register our component as xtype, but we really don't need that
Ext.reg('termstree', Neologism.TermsTree);

/**  
 * Add observer TermsTree component that its data depend of this one  
 * @return nothing  
 */   
Neologism.TermsTree.prototype.addObserver = function(observer){   
    if( typeof observer == 'undefined' ) {   
        return;
    }   
   
    //this.relayEvents(observer, ['selectionchange']);
    this.observers.push(observer);
};  

/**  
 * Iterate over the observers array and fire the event for each component  
 * @return nothing  
 */   
Neologism.TermsTree.prototype.notifyObservers = function(event, object){   
	if( typeof event == 'undefined' ) {   
        return;
    }  
	
	for( var i = 0; i < this.observers.length; i++ ) {   
		this.observers[i].fireEvent(event, object);
    } 
};

/**
 * 
 */
Neologism.TermsTree.prototype.findNodeByText = function(text){   
    var node = null;
	this.getRootNode().eachChild(function(currentNode){
        currentNode.cascade(function(){
        	if( this.attributes.text == text ) {
        		node = this;
        		return false;
        	}
        }, null);
        
        if( node !== null ) {
        	//node.setOwnerTree(this.getOwnerTree());
        	return false;
        }
        
    },  null);
	
	return node;
};

/**
 * Compute the posible inverse properties for a property with domain "domain" and range "range"
 * 
 * @param rootNodeClasses current classes tree structure.
 * @param domain current domain for the property whose inverse going to be computed.
 * @param range current range for the property whose inverse going to be computed.
 * @return an array containing all the allowed as inverse properties
 */
Neologism.TermsTree.prototype.computeInverses = function(rootNodeClasses, domain, range){   
	// TODO: all parameter are obligatory, so we need to check for them.
	
	// Domain
	// add all classes that are disjoint with domain
	// and also add all classes that are disjoint with any superclass of domain
	var domainSet = [];
	for ( var i = 0; i < domain.length; i++ ) {
		var node = rootNodeClasses.getOwnerTree().findNodeByText(domain[i]);
		if( node != null ) {
			// this could be solved mean this way but I have to fix some bug that I found
			// regarding some class was repeated
			//domainSet = domainSet.concat(node.attributes.disjointwith);
			var d = node.attributes.disjointwith;
			for ( var j = 0; j < d.length; j++ ) {
				if ( domainSet.indexOf(d[j]) == -1 ) {
					domainSet.push(d[j]);
				}
			}
		}
	}
	
	// include also all subclasses of any of the classes added above
	var finalDomainSet = [];
	for ( var i = 0; i < domainSet.length; i++ ) {
		var node = rootNodeClasses.getOwnerTree().findNodeByText(domainSet[i]);
		if( node != null ) {
			// all child also are disjoint
			node.eachChild( function(currentNode) {
	            currentNode.cascade( function() {
	            	if ( finalDomainSet.indexOf(this.text) == -1 ) {
	            		finalDomainSet.push(this.text);
					}
	            }, null);
	        });
		}
	}
	
	finalDomainSet = finalDomainSet.concat(domainSet);
	
	// Range
	// add all classes that are disjoint with range
	// and also add all classes that are disjoint with any superclass of range
	var rangeSet = [];
	for ( var i = 0; i < range.length; i++ ) {
		var node = rootNodeClasses.getOwnerTree().findNodeByText(range[i]);
		if( node != null ) {
			// this could be solved mean this way but I have to fix some bug that I found
			// regarding some class was repeated
			//domainSet = domainSet.concat(node.attributes.disjointwith);
			var d = node.attributes.disjointwith;
			for ( var j = 0; j < d.length; j++ ) {
				if ( rangeSet.indexOf(d[j]) == -1 ) {
					rangeSet.push(d[j]);
				}
			}
		}
	}
	
	// include also all subclasses of any of the classes added above
	var finalRangeSet = [];
	for ( var i = 0; i < rangeSet.length; i++ ) {
		var node = rootNodeClasses.getOwnerTree().findNodeByText(rangeSet[i]);
		if( node != null ) {
			// all child also are disjoint
			node.eachChild( function(currentNode) {
	            currentNode.cascade( function() {
	            	if ( finalRangeSet.indexOf(this.text) == -1 ) {
	            		finalRangeSet.push(this.text);
					}
	            }, null);
	        });
		}
	}
	
	finalRangeSet = finalRangeSet.concat(rangeSet);
	
	// check for rules
	// A property i is allowed as an inverse of p if it fulfills all of the conditions below:
	
	// The domain of i is not any of the classes in finalRangeSet
	var allowedAsInverseProperties = [];
	// traverse the tree
	this.getRootNode().eachChild(function(currentNode){
        currentNode.cascade(function(){
        	
        	var allowed = true;
        	
        	var d = this.attributes.domain;
        	for ( var i = 0; i < d.length; i++ ) {
        		// The domain of i=this is not any of the classes in finalRangeSet
        		if ( finalRangeSet.indexOf(d[i]) != -1 ) {
        			allowed = false;
        			break;
				}
        	}
        	
        	if( allowed ) {
	        	var r = this.attributes.range;
	        	var rangeHasOnlyLiterals = true;
	        	for ( var i = 0; i < r.length; i++ ) {
	        		// The range of this is not any of the classes in finalDomainSet
	        		if ( finalDomainSet.indexOf(r[i]) != -1 ) {
	        			allowed = false;
	        			break;
					}
	        		
	        		if ( rangeHasOnlyLiterals ) {
	        			if( Neologism.TermsTree.getXSDDatatype().indexOf(r[i]) == -1 ) {
	        				rangeHasOnlyLiterals = false;
	        			}
	        		}
	        	}
	        	
	        	// if this only contain literal it is not allowed
	        	if ( rangeHasOnlyLiterals ) {
	        		allowed = false;
	        	}
        	}
        	
        	// if the property was allowed as an inverse all its superproperties also fulfill the conditions
        	if( allowed ) {
        		// add this to the resulting array
        		if ( allowedAsInverseProperties.indexOf(this.text) == -1 ) {
        			allowedAsInverseProperties.push(this.text);
				}
        		
        		if ( !this.parentNode.isRoot ) {
	            	this.bubble( function() {
	            		if ( allowedAsInverseProperties.indexOf(this.text) == -1 ) {
	            			allowedAsInverseProperties.push(this.text);
	    				}
		                // if this node is the root node then return false to stop the bubble process
		                if ( this.parentNode.isRoot ) {
		                	return false;
		                }
	            	});
	            }
        	}
        	
        }, null);
        
    },  null);
	
	return allowedAsInverseProperties;
}

/**
 * Finds the first child that has the attribute with the specified value using the children array.
 * @param {Node} node containing children
 * @param {String} attribute The attribute name
 * @param {Mixed} value The value to search for
 * @return {Node} The found child or null if none was found
 */
Neologism.TermsTree.prototype.findChildInNode = function(node, value) { 
	var cs = node.attributes.children;
    for(var i = 0, len = cs.length; i < len; i++) {
        if(cs[i].text == value){
            return cs[i];
        }
    }
    return null;
}

/**
 * Static method that return the xsd datatype.
 * This is a fast solution, but the idea is to update that list from the server because now
 * the developer should update both function and this could bring some errors.
 */
Neologism.TermsTree.getXSDDatatype = function() {
	return [
				'rdfs:Literal',
				'xsd:string',
				'xsd:boolean',
				'rdf:XMLLiteral',	
				'xsd:date',
				'xsd:dateTime',
				'xsd:time',
				'xsd:gYearMonth',
				'xsd:gYear',
				'xsd:gMonthDay',
				'xsd:time',
				'xsd:gDay',
				'xsd:gMonth',
				'xsd:decimal',
				'xsd:float',
				'xsd:double',
				'xsd:integer',
				'xsd:nonPositiveInteger',
				'xsd:negativeInteger',
				'xsd:long',
				'xsd:int',
				'xsd:short',
				'xsd:byte',
				'xsd:nonNegativeInteger',
				'xsd:unsignedLong',
				'xsd:unsignedInt',
				'xsd:unsignedShort',
				'xsd:unsignedByte',
				'xsd:unsignedInt',
				'xsd:hexBinary',
				'xsd:base64Binary',
				'xsd:anyURI',
				'xsd:normalizedString',
				'xsd:token',
				'xsd:language',
				'xsd:NMTOKEN',
				'xsd:Name',
				'xsd:NCName'
				];
};



