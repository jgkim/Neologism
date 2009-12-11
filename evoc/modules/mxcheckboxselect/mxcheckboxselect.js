/**
 * @author guicec
 */

var EvocWidget = {};

/**
* appends a javascript array to a form
*
* @param array array - the array
* @param string name - name of the array 
* @param mixed form - the form
*/
EvocWidget.convertJsArrayToPhpArray = function( array, name, form ) {
  
  if ( typeof( form ) == 'string' ) {
    form = document.getElementById( form );
  }
  
  var hidden = null;
  for( index = 0; index < array.length; index++ ) {
    hidden = document.createElement( 'input' );
    hidden.setAttribute( 'type', 'hidden' );
    hidden.setAttribute( 'name', name + '[' + index +']' );
    hidden.setAttribute( 'value', array[index]);
    form.appendChild( hidden );
  }
  
  return true;
}

/**
 * This function create input field for all the Drupal.settings.neologism.field_values
 * 
 * @param {Object} formId
 */
EvocWidget.onsubmitCreateInputFields = function(formId){

  //alert('onsubmitCreateInputFields(' + formId.toString() + ')');
 
  for (field in Drupal.settings.neologism.field_values ) {
    //alert(field);
    // it's very important know that we are using field.toString() + "_values" to hold the values.
    // so, we need to access it from the server side (PHP) as $field_name."_values"
    //alert(Drupal.settings.neologism.field_values[field]);
    EvocWidget.convertJsArrayToPhpArray(
      Drupal.settings.neologism.field_values[field], 
      field.toString() + "_values", 
      formId
    );    
  }
  
  //alert('before return: ' + formId.toString());
  return true;
}

/**
 * 
 * @param {Object} field_name
 */ 
EvocWidget.createStandardClassSelecctionWidget = function( field_name ) {
  
  var objectToRender = Drupal.settings.neologism.field_id[field_name];
  var dataUrl = Drupal.settings.neologism.json_url[field_name];
  var editingValue = Drupal.settings.neologism.editing_value[field_name];
   
  // we need to past the baseParams as and object, that is why we creat the baseParams object
  // and add the arrayOfValues array 
  var baseParams = {};
  Drupal.settings.neologism.field_values[field_name] = Ext.util.JSON.decode(Drupal.settings.neologism.field_values[field_name]);
  baseParams.arrayOfValues = Drupal.settings.neologism.field_values[field_name];
 
  var termsTree = new Neologism.TermsTree({
    renderTo: objectToRender,
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
            node.getOwnerTree().expandPath(currentNode.getPath());
            currentNode.cascade( function() {
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
    
    // SET the root node.
    root: new Ext.tree.AsyncTreeNode({
      text	: Drupal.t('Thing / Superclass'),
      id		: 'root',                  // this IS the id of the startnode
      iconCls: 'class-samevoc',
      disabled: true,
      expanded: false,
    }),
  
    listeners: {
      // behaviour for on checkchange in superclassesTree TreePanel object 
      checkchange: function(node, checked) {
        if ( checked && node.parentNode !== null ) {
          // if we're checking the box, check it all the way up
    			if ( node.parentNode.isRoot || !node.parentNode.getUI().isChecked() ) {
            if ( baseParams.arrayOfValues.indexOf(node.id) == -1 ) {
              baseParams.arrayOfValues.push(node.id);
            }
          }
    		} else {
          for (var i = 0, len = baseParams.arrayOfValues.length; i < len; i++) {
            if ( baseParams.arrayOfValues[i] == node.attributes.id ) {
              baseParams.arrayOfValues.splice(i, 1);
            }
          }    
        }
        
        node.getOwnerTree().expandPath(node.getPath());
        node.cascade( function(){
          this.expand();
          //alert(this.id + " == " + editingValue + " result = " + (this.id == editingValue) );
          if ( this.id != editingValue ) {
            if ( this.id != node.id ) {
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
    }
  });
}
