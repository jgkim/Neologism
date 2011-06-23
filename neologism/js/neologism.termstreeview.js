Neologism.TermsTreeView = Ext.extend(Ext.tree.TreePanel, {
      height           : 500,
      autoHeight       : true,
      rootVisible      : false,
      maxPathDepth	   : 3,

	  initComponent: function() {
	    // Called during component initialization
	    var config = {
	      //props (non-overridable)
	      
	      //------------------------------------------- standard TreePanel properties
	      useArrows        : true,  
	      collapsible      : false,
	      animCollapse     : true,
	      border           : true,
	      autoScroll       : true,
	      animate          : false,
	      containerScroll  : true,
	      enableDD         : false,
	      singleClickExpand: true

	      //------------------------------------------- 
	      // standard TreePanel properties
	    };
	  
	    // Config object has already been applied to 'this' so properties can 
	    // be overriden here or new properties (e.g. items, tools, buttons) 
	    // can be added, eg:
	    Ext.apply(this, config);
	    Ext.apply(this.initialConfig, config);

	    // Call parent (required)
	    Neologism.TermsTree.superclass.initComponent.apply(this, arguments);
	    
	    this.addEvents('fullexpanded');
	    
	    this.on('click', function(node, e) {
			e.stopEvent();
			var term = node.text.split(":"); 
			window.location = '#' + term[1].toString();
			return false;
		 });
	}
});

//this to register our component as xtype, but we really don't need that
Ext.reg('termstreeview', Neologism.TermsTreeView);