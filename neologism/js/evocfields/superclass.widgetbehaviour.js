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
    		var treePanel = node.getOwnerTree();
    		Neologism.TermsTree.traverse(node, function(currentNode, path) {
    			if( Neologism.util.in_array(currentNode.text, baseParams.arrayOfValues) ) {
    				treePanel.expandPath(path.join('/'));
    			}
    		}, true);
        }
      }
    }),
    
    root: new Ext.tree.AsyncTreeNode({
      text	: Drupal.t('Thing / Superclass'),
      id	: 'root',                  // this IS the id of the startnode
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
	        	for ( var i = 0, len = baseParams.arrayOfValues.length; i < len; i++ ) {
	        		if ( baseParams.arrayOfValues[i] == node.text ) {
	        			baseParams.arrayOfValues.splice(i, 1);
	        		}
	        	}
	        }
	        
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

