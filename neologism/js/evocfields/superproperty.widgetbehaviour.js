/**
 * 
 * @param {Object} field_name
 */
Neologism.createSuperpropertySelecctionWidget = function(field_name) {

  var objectToRender = Drupal.settings.evocwidget.field_id[field_name];
  var dataUrl = Drupal.settings.evocwidget.json_url[field_name];
  var editingValue = Drupal.settings.evocwidget.editing_value[field_name];
  
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  //Drupal.settings.neologism.field_values[field_name] = Drupal.parseJson(Drupal.settings.neologism.field_values[field_name]);
  Drupal.settings.evocwidget.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.evocwidget.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.evocwidget.field_values[field_name];
  
  Neologism.superpropertyTermsTree = new Neologism.TermsTree({
    //renderTo: objectToRender,
    title: Drupal.t('Subproperty of'),
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
            if ( currentNode !== undefined ) {
        	  
        	  currentNode.cascade(function(){
        		  this.expand();
	              if (this.text == editingValue) {
	            	  this.remove();
	              }
	              
	              for (var j = 0, lenValues = baseParams.arrayOfValues.length; j < lenValues; j++) {
	                if (this.text == baseParams.arrayOfValues[j]) {
	                  this.getUI().toggleCheck(true);
	                }
	              }
        	  	}, null);
            }
          });
        }
    
      }
    }),
    
    root: new Ext.tree.AsyncTreeNode({
      text: Drupal.t('Thing / Superclass'),
      id: 'root', // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false
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
  
  Neologism.superpropertyTermsTree.objectToRender = objectToRender;
};
    