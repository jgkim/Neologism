/**
 * @author guicec
 */

Ext.ns('Neologism');

/**
 * Override TreePanel onClick and onDblClick events
 * @param {Object} e
 */ 
Ext.override(Ext.tree.TreeNodeUI, {
  onClick : function(e) { //debugger;
    if ( this.dropping ){
      e.stopEvent();
      return;
    }
    if ( this.fireEvent("beforeclick", this.node, e) !== false ) {
      var a = e.getTarget('a');
      if ( !this.disabled && this.node.attributes.href && a ){
        this.fireEvent("click", this.node, e);
        return;
      }else if ( a && e.ctrlKey ){
        e.stopEvent();
      }
      e.preventDefault();
      if(this.disabled){
        return;
      }
      if( this.node.attributes.singleClickExpand && !this.animating && this.node.hasChildNodes() ){
        //this.node.expand(); 
        //this.node.toggle();
      }
  
      this.fireEvent("click", this.node, e);
    }else {
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

Neologism.TermsTree = Ext.extend(Ext.tree.TreePanel, {
  
  //props (overridable by caller)
  height           : 400,
  width            : '100%',
  disabled         : false,
  rootVisible      : false,
  header           : false,
  headerAsText     : false, // hidden the header title

  initComponent: function(){
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
      hiddenNodes: []
      
    };
  
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
  },

  refresh:function(){
    this.loader.load( this.getRootNode() );
  }

});

// this to register our component as xtype, but we really don't need that
Ext.reg('termstree', Neologism.TermsTree);

