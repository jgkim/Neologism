/**
 * @author guicec
 */
$(document).ready( function() {
    var pathsToExpand = new Array();
    // Define Tree.
    var treeLoader = new Ext.tree.TreeLoader({
        dataUrl   : Drupal.settings.neologism.json_url,
        baseParams: Drupal.settings.neologism,
        listeners: {
            load: function(loader, node, response){
    			var treePanel = node.getOwnerTree();
				for (var i = 0; i < pathsToExpand.length; i++) {
					treePanel.expandPath(pathsToExpand[i]);
				}
				
				treePanel.fireEvent('fullexpanded', {
					name: 'classesTreeViewPanel',
		    		newHeight: treePanel.getHeight() - treePanel.header.getHeight() - 1
				});
				
				pathsToExpand.length = 0;
		    }
          }
    });
    
    // SET the root node.
    var rootNode = new Ext.tree.AsyncTreeNode({
        text	: Drupal.t('Thing/Superclass'), 
        id		: 'root',                  // this IS the id of the startnode
        iconCls: 'class-samevoc',
        listeners: {
    		beforeexpand: function( /*Node*/ node, /*Boolean*/ deep, /*Boolean*/ anim ) {
			    var treePanel = node.getOwnerTree();
				Neologism.TermsTree.traverse(node, function(currentNode, path) {
					var pathToExpand = path.slice();
					if (pathToExpand.length > treePanel.maxPathDepth+1) {
						return;
					}
  					pathToExpand.pop();
					var path = pathToExpand.join('/');
					if( !Neologism.util.in_array(path, pathsToExpand)) {
						pathsToExpand.push(path);
					}
					lastNodeName = currentNode.text; 
				}, true);
    		}
    	}
    });
    
    Neologism.classesTreeViewPanel = new Neologism.TermsTreeView({
        title            : Drupal.t('Classes'),
        id               : "class-tree",
        el               : "class-tree",
        loader           : treeLoader,
        root             : rootNode,
        rootVisible      : false,
    });
    
    Neologism.classesTreeViewPanel.render();
	Neologism.classesTreeViewPanel.on('fullexpanded', Drupal.neologism.checkTreeViewsHeight);
});