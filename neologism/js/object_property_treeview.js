/**
 * @author guicec
 */
Ext.onReady(function() {
	
    // Define Tree.
  var treeLoader = new Ext.tree.TreeLoader({
    dataUrl   : Drupal.settings.neologism.property_json_url,
    baseParams: Drupal.settings.neologism
    });
    
    // SET the root node.
  var rootNode = new Ext.tree.AsyncTreeNode({
        //text	: Drupal.t('Thing / Superclass'),
    id		: 'super'                  // this IS the id of the startnode
    });
    
  var objectPropertyTree = new Ext.tree.TreePanel({
    title            : Drupal.t('Properties'),
    useArrows        : true,  
    collapsible      : false,
    animCollapse     : true,
    border           : true,
    id               : "object-property-tree",
    el               : "object-property-tree",
    autoScroll       : true,
    animate          : false,
    enableDD         : false,
    containerScroll  : true,
    height           : 200,
    autoHeight       : true,
    width            : '100%',
    loader           : treeLoader,
    root             : rootNode,
    rootVisible      : false,
    
    listeners: {
      click: function(node, e){
        var term = node.id.split(":"); 
        window.location = '#' + term[1].toString();
        //Ext.Msg.alert('Info!', term[1].toString());  
        //Ext.Msg.alert('Info!', term[1].toString());
        return false;
      }
    },
    
    /*
    tbar: {
            cls:'top-toolbar',
            items:[' ',
			        {
                xtype: 'tbbutton',
                iconCls: 'icon-expand-all', 
                tooltip: Drupal.t('Expand all'),
                handler: function(){ 
                  rootNode.expand(true); 
                }
              }, {
                xtype: 'tbseparator' // equivalent to '-'
              }, {
                iconCls: 'icon-collapse-all',
                tooltip: Drupal.t('Collapse all'),
                handler: function(){ 
                  rootNode.collapse(true); 
                }
              }
            ]
        }
       */
    });
 
    // Render the tree.
    //Tree_Category.setRootNode(Tree_Category_Root);
  objectPropertyTree.render();
  objectPropertyTree.getRootNode().expand(true, false);
 
  
});