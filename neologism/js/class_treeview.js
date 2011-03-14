/**
 * @author guicec
 */
Ext.onReady(function() {
	
    //Ext.QuickTips.init();
    
    // Define Tree.
    var treeLoader = new Ext.tree.TreeLoader({
        dataUrl   : Drupal.settings.neologism.json_url,
        baseParams: Drupal.settings.neologism
    });
    
    // SET the root node.
    var rootNode = new Ext.tree.AsyncTreeNode({
        text	: Drupal.t('Thing / Superclass'),
        id		: 'super',                  // this IS the id of the startnode
        iconCls: 'class-samevoc'
    });
    
    var tree = new Ext.tree.TreePanel({
        title            : Drupal.t('Classes'),
        id               : "class-tree",
        el               : "class-tree",
        useArrows        : true,  
        collapsible      : false,
        animCollapse     : true,
        border           : true,
        autoScroll       : true,
        animate          : false,
        enableDD         : false,
        containerScroll  : true,
        height           : 400,
        autoHeight       : true,
        //width            : '100%',
        loader           : treeLoader,
        root             : rootNode,
        rootVisible      : false,
        
        listeners: {
          click: function(node, e){
            e.stopEvent();
            var term = node.id.split(":"); 
            window.location = '#' + term[1].toString();
            return false;
          }
        }
    });
    
    tree.render();
    tree.getRootNode().expand(true, false);
 
});