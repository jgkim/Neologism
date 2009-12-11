/**
 * @author guicec
 */

/**
 * Create the class selection widget behaviour for filed_superclass2 
 * 
 * @param {Object} field_name
 */
Neologism.createClassSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.neologism.field_id[field_name];
  var dataUrl = Drupal.settings.neologism.json_url[field_name];
  var editingValue = Drupal.settings.neologism.editing_value[field_name];
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.neologism.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.neologism.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.neologism.field_values[field_name];

  Neologism.superclassesTreePanel = new Neologism.TermsTree({
    renderTo: objectToRender,
    title: Drupal.t('Classes'),
    disabled: true,
    
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
              // expand the node to iterate over it
              this.getOwnerTree().expandPath(this.getPath());
              
              if ( this.id == editingValue ) {
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
          
          Neologism.disjointWithTreePanel.render(Neologism.objectToRender);
        }
      }
    }),
    
    root: new Ext.tree.AsyncTreeNode({
      text	: Drupal.t('Thing / Superclass'),
      id		: 'root',                  // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false,
    }),
    
    listeners: {
      // behaviour for on checkchange in Neologism.superclassesTree TreePanel object 
      checkchange: function(node, checked) {
        if ( checked && node.parentNode !== null ) {
          // if we're checking the box, check it all the way up
    			if ( node.parentNode.isRoot || !node.parentNode.getUI().isChecked() ) {
            
            //Ext.Msg.alert('Checkbox status', 'Checked: "' + node.attributes.text);
            //Neologism.classSelection.push(node.id);
            //alert(node.id);
            if ( baseParams.arrayOfValues.indexOf(node.id) == -1 ) {
              baseParams.arrayOfValues.push(node.id);
            }
            
            if ( !node.parentNode.isRoot ) {
              node.bubble( function(){
                if (node.id != this.id && node.getUI().nodeClass != 'locked-for-edition' ) {
                  this.getUI().checkbox.disabled = true;
                }
                //this.getUI().addClass('complete');
                // if this node is the root node then return false to stop the bubble process
                if ( this.parentNode.isRoot ) {
                  return false;
                }
              });
            }
            
            //alert(Neologism.disjointWithTree);
            // disable all the classes in disjoint tree
            Neologism.disjointWithTreePanel.expandPath(node.getPath());
            disjointWithNode = Neologism.disjointWithTreePanel.getNodeById(node.id);
            // when the Neologism.superclassesTree is expanding its nodes the
            // Neologism.disjointWithTree has not load its nodes yet, so we need to check it
            // to avoid errors 
            //alert(disjointWithNode);
            if( disjointWithNode !== undefined ) {
              disjointWithNode.bubble( function(){
                this.getUI().checkbox.disabled	= true;
                this.getUI().addClass('complete');
                // if this node is the root node then return false to stop the bubble process
                if ( this.parentNode.id == Neologism.disjointWithTreePanel.getRootNode().id ) {
                  return false;
                }
      				});
            }
          }
    		} else {
          for ( var i = 0, len = baseParams.arrayOfValues.length; i < len; i++ ) {
            if ( baseParams.arrayOfValues[i] == node.attributes.id ) {
              
              //alert(node.getPath());
              baseParams.arrayOfValues.splice(i, 1);
              
              if (!node.parentNode.isRoot) {
                node.bubble( function(){
                  if (node.id != this.id && node.getUI().nodeClass != 'locked-for-edition') {
                    this.getUI().checkbox.disabled = false;
                  }
                  //this.getUI().addClass('complete');
                  // if this node is the root node then return false to stop the bubble process
                  if ( this.parentNode.isRoot ) {
                    return false;
                  }
                });
              }
              
              // enable all the classes in disjoint tree
              Neologism.disjointWithTreePanel.expandPath(node.getPath());
              Neologism.disjointWithTreePanel.getNodeById(node.id).bubble( function(){
                this.getUI().checkbox.disabled = false;
                this.getUI().removeClass('complete');
                
                // stop the bubble if the parent is the root node
                if ( this.parentNode.id == Neologism.disjointWithTreePanel.getRootNode().id ) {
                  return false;
                }
                
                // Loop through its childen
                for (var i = 0, len = this.parentNode.childNodes.length; i < len; i++) {
                  var currentChild = this.parentNode.childNodes[i];
                              
                  // if this child is disable so we need to keep its parent disable, return false to stop
                  // bubble process
                  if ( currentChild.getUI().checkbox.disabled == true ) {
                    return false;
                  }
                }
    				  });
             
            }
          }    
        }

        node.cascade( function(){
          this.expand();
          //alert(this.id + " == " + editingValue + " result = " + (this.id == editingValue) );
          if ( this.id != editingValue ) {
            if (this.id != node.id && this.getUI().nodeClass != 'locked-for-edition' && this.getUI().nodeClass != 'complete') {
              this.getUI().checkbox.disabled = node.getUI().checkbox.checked;
            }
          }
          else if ( this.id == editingValue ) {
            this.getUI().addClass('locked-for-edition');
            this.getUI().checkbox.disabled = true;
            this.getUI().checkbox.checked = false;
          }
        });
        
      } // checkchange  
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

}

/**
 * widget behaviour for field_disjointwith2 field
 * 
 * @param {Object} field_name
 */
Neologism.createDisjointWithSelecctionWidget = function(field_name){
  var objectToRender = Drupal.settings.neologism.field_id[field_name];
  var editingValue = Drupal.settings.neologism.editing_value[field_name];
  Neologism.objectToRender = objectToRender;
  
  var dataUrl = Drupal.settings.neologism.json_url[field_name];
  
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.neologism.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.neologism.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.neologism.field_values[field_name];
  
  Neologism.disjointWithTreePanel = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Disjoint with class(es)'),
    disabled: true,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,//baseParams,
      listeners: {
        load: function(loader, node, response){
          node.eachChild(function(currentNode){
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
          
          // enable disjointwith treepanel
          node.getOwnerTree().enable();
          // refresh the superclasswaTreePanel to synchronize nodes between both treepanel
          Neologism.superclassesTreePanel.updatselection();
          // enable superclasses treepanel
          Neologism.superclassesTreePanel.enable();
        }
      }
    }),
    
    // SET the root node.
    root: new Ext.tree.AsyncTreeNode({
      text: Drupal.t('Thing / Superclass'),
      id: 'root', // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false,
    }),
    
    listeners: {
      checkchange: function(node, checked){
        if (checked && node.parentNode !== null) {
          // if we're checking the box, check it all the way up
          if (node.parentNode.isRoot || !node.parentNode.getUI().isChecked()) {
            if (baseParams.arrayOfValues.indexOf(node.id) == -1) {
              baseParams.arrayOfValues.push(node.id);
            }
            
            // disable all the classes in disjoint tree
            Neologism.superclassesTreePanel.expandPath(node.getPath());
            superclassesTreeNode = Neologism.superclassesTreePanel.getNodeById(node.id);
            if (superclassesTreeNode !== undefined) {
              superclassesTreeNode.getUI().checkbox.disabled = true;
              superclassesTreeNode.getUI().nodeClass = 'complete';
              superclassesTreeNode.getUI().addClass('complete');
              
              superclassesTreeNode.eachChild(function(currentNode){
                Neologism.superclassesTreePanel.expandPath(currentNode.getPath());
                currentNode.cascade(function(){
                  //alert(this);
                  this.getUI().checkbox.disabled = true;
                  this.getUI().nodeClass = 'complete';
                  this.getUI().addClass('complete');
                }, null);
              });
            }
            
          }
        }
        else {
          for (i in baseParams.arrayOfValues) {
            if (baseParams.arrayOfValues[i] == node.attributes.id) {
              baseParams.arrayOfValues.splice(i, 1);
              
              // enable all the classes in disjoint tree
              Neologism.superclassesTreePanel.expandPath(node.getPath());
              superclassesTreeNode = Neologism.superclassesTreePanel.getNodeById(node.id);
              if (!superclassesTreeNode.parentNode.getUI().isChecked()) {
                superclassesTreeNode.getUI().checkbox.disabled = false;
              }
              superclassesTreeNode.getUI().nodeClass = '';
              superclassesTreeNode.getUI().removeClass('complete');
              
              superclassesTreeNode.eachChild(function(currentNode){
                Neologism.superclassesTreePanel.expandPath(currentNode.getPath());
                currentNode.cascade(function(){
                  //alert(this);
                  this.getUI().checkbox.disabled = false;
                  this.getUI().nodeClass = '';
                  this.getUI().removeClass('complete');
                }, null);
              });
              
            }
          }
        }
        
        //node.getOwnerTree().expandPath(node.getPath());
        node.cascade(function(){
          this.expand();
          //alert(this.id);
          if (this.id == editingValue) {
            this.getUI().addClass('locked-for-edition');
            this.getUI().checkbox.disabled = true;
            this.getUI().checkbox.checked = false;
          }
          else {
            if (this.id != node.id) {
              this.getUI().checkbox.disabled = node.getUI().checkbox.checked;
            }
          }
          
        });
        
      }
    } // listeners
   });
}

/**
 * 
 * @param {Object} field_name
 */
Neologism.createSuperpropertySelecctionWidget = function(field_name){

  var objectToRender = Drupal.settings.neologism.field_id[field_name];
  var dataUrl = Drupal.settings.neologism.json_url[field_name];
  var editingValue = Drupal.settings.neologism.editing_value[field_name];
  
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.neologism.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.neologism.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.neologism.field_values[field_name];
  
  var termsTree = new Neologism.TermsTree({
    renderTo: objectToRender,
    title: Drupal.t('Subproperty of'),
    disabled: true,
    
    loader: new Ext.tree.TreeLoader({
      dataUrl: dataUrl,
      baseParams: baseParams,
      
      listeners: {
        // load : ( Object This, Object node, Object response )
        // Fires when the node has been successfuly loaded.
        // added event to refresh the checkbox from its parent 
        load: function(loader, node, response){
        
          node.eachChild(function(currentNode){
            //alert(currentNode.id);
            //node.getOwnerTree().expandPath(currentNode.getPath());
            currentNode.expand();
            currentNode.cascade(function(){
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
          
          node.getOwnerTree().enable();
        }
      }
    }),
    
    root: new Ext.tree.AsyncTreeNode({
      text: Drupal.t('Thing / Superclass'),
      id: 'root', // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false,
    }),
    
    listeners: {
      // behaviour for on checkchange in Neologism.superclassesTree TreePanel object 
      checkchange: function(node, checked){
        if (checked && node.parentNode !== null) {
          // if we're checking the box, check it all the way up
          if (node.parentNode.isRoot || !node.parentNode.getUI().isChecked()) {
            if (baseParams.arrayOfValues.indexOf(node.id) == -1) {
              baseParams.arrayOfValues.push(node.id);
            }
          }
        }
        else {
          for (var i = 0, len = baseParams.arrayOfValues.length; i < len; i++) {
            if (baseParams.arrayOfValues[i] == node.attributes.id) {
              //alert(node.getPath());
              baseParams.arrayOfValues.splice(i, 1);
            }
          }
        }
        
        node.getOwnerTree().expandPath(node.getPath());
        node.cascade(function(){
          this.expand();
          
          if (this.id == editingValue) {
            this.getUI().addClass('locked-for-edition');
            this.getUI().checkbox.disabled = true;
            this.getUI().checkbox.checked = false;
          }
          else {
            if (this.id != node.id) {
              this.getUI().checkbox.disabled = node.getUI().checkbox.checked;
            }
          }
        });
      } // checkchange
    }
  });
}
    
    