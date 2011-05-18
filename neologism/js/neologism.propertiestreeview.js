/**
 * @author guicec
 */
$(document).ready( function() {
	var pathsToExpand = new Array();
	// Define Tree.
	var treeLoader = new Ext.tree.TreeLoader({
	    dataUrl   : Drupal.settings.neologism.property_json_url,
	    baseParams: Drupal.settings.neologism,
	    listeners: {
	        load: function(loader, node, response){
				var treePanel = node.getOwnerTree();
				for (var i = 0; i < pathsToExpand.length; i++) {
					treePanel.expandPath(pathsToExpand[i]);
				}
				
				treePanel.fireEvent('fullexpanded', {
					name: 'propertiesTreeViewPanel',
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
	    iconCls: 'property-samevoc',
	    listeners: {
			beforeexpand: function( /*Node*/ node, /*Boolean*/ deep, /*Boolean*/ anim ) {
	  	    	//if (treeloaded) return;
			    var treePanel = node.getOwnerTree();
				Neologism.TermsTree.traverse(node, function(currentNode, path) {
					var pathToExpand = path.slice();
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

	Neologism.propertiesTreeViewPanel = new Neologism.TermsTreeView({
		title            : Drupal.t('Properties'),
	    id               : "object-property-tree",
	    el               : "object-property-tree",
	    loader           : treeLoader,
	    root             : rootNode,
	    rootVisible      : false
	});
	
	Neologism.propertiesTreeViewPanel.render();
	Neologism.propertiesTreeViewPanel.on('fullexpanded', Drupal.neologism.checkTreeViewsHeight);
});