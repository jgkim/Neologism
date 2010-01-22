/**
 * Neologism Simple Visualizer version 1.3
 * 
 */
package vis.vocabulary.display
{
	import flash.display.BitmapData;
	import flash.display.DisplayObject;
	import flash.display.GradientType;
	import flash.display.SpreadMethod;
	import flash.events.Event;
	import flash.geom.Point;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	
	import mx.containers.Canvas;
	import mx.core.Container;
	
	public class VocabularyCanvas extends Canvas
	{		
		public static var VOCABULARY_READY:String = "vocabulary_ready";
		private static var PROPERTY_STYLE:Array = [2,0x73d216,1];
		private static var SUBCLASS_STYLE:Array = [2,0xcc0000,1];
		
		private static var GRID_STYLE:Array = [1,0xd3d7cf,0.7];
		// dashed style or not
		private static var DOMAIN_DASH_STYLE:Array = [GradientType.LINEAR,
													[PROPERTY_STYLE[1],PROPERTY_STYLE[1]],
													[0,100],
													[0,255],
													null,
													SpreadMethod.PAD];
															
		private var _showGrid:Boolean;
		private var _renderNodesShadow:Boolean = true;
		private var _gridSpaceing:Number;
		private var _loader:URLLoader;
		
		private var _vocabularyURL:String;
		private var _layoutURL:String
		
		//private var _properties:Object;
		//private var _classes:Object;
		private var _classes:Array;
		private var _properties:Array;
		private var _arcs:Array;
		private var _rdfNodes:Array;
		
		// this attr control whether an arc is current selected
		private var someArcSelected:Boolean = false;
		private var selectedArc:RDFArc = null;		
		
		public function VocabularyCanvas()
		{
			super();
			this._showGrid = true;
			this._gridSpaceing = 17;
			//this._arcDeviation = 45;
			//this._properties = {};
			//this._classes = {};
			_classes = new Array();
			_properties = new Array();
			this._arcs = [];
			
			addEventListener(RDFArc.ARC_SELECTED, this.onSelectArc);
			addEventListener(RDFArc.ARC_DESELECTED, this.onDeselectArc);
			addEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, true);
		}
		
		public function get showGrid(): Boolean {
			return this._showGrid;
		}
		
		public function set showGrid(value:Boolean):void {
			this._showGrid = value;
		}
		
		public function get gridSpacing() : Number
		{
			return this._gridSpaceing;
		}
		
		public function set gridSpacing(gs:Number) : void
		{
			this._gridSpaceing = gs;
			this.updateDisplayList(unscaledWidth,unscaledHeight);
		}
		
		/*
		public function get arcDeviation() : Number
		{
			return this._arcDeviation;
		}
		
		public function set arcDeviation(ad:Number) : void
		{
			this._arcDeviation = ad;
		}
		*/
		
		private function paintGrid() : void
		{
			if( _showGrid ) {
				this.graphics.lineStyle(GRID_STYLE[0], GRID_STYLE[1], GRID_STYLE[2]);
				var w:Number = this.width / this.scaleX;
				var h:Number = this.height / this.scaleY;
				
				for (var i:Number= _gridSpaceing; i <= w; i += _gridSpaceing) {
					this.graphics.moveTo(i, this.y);
					this.graphics.lineTo(i, this.y + h);
				}
				
				for (i = _gridSpaceing; i <= h; i += _gridSpaceing) {
					this.graphics.moveTo(this.x, i);
					this.graphics.lineTo(this.x + w, i);
				}
			}
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void
		{
			this.graphics.clear();
			
			paintGrid();
			
			for each (var arc:RDFArc in _arcs) {
				 arc.draw();
			}
			
			super.updateDisplayList(unscaledWidth, unscaledHeight);
		}
		
		/**
		 * Create the RDFNode and the RDFObjectHandles to manipulete it as a selectable object
		 *  
		 */
		public function addRdfClass(rdfName:String, rdfLabel:String, external:Boolean): RDFNode
		{
			var rdfClass:RDFNode= new RDFNode(rdfName, rdfLabel, external);
			rdfClass.renderShadow = _renderNodesShadow;
			
			// create the wrapper to hold the component
			var rdfObjectHandle:RDFObjectHandles = new RDFObjectHandles();
			rdfObjectHandle.allowRotate = false;
			//rdfObjectHandle.allowVMove= false;
			rdfObjectHandle.mouseCursors = null;//new ObjectHandlesMouseCursors2();
			
			rdfObjectHandle.addChild(rdfClass);
			rdfObjectHandle.width = 100;
			rdfObjectHandle.height = 28;
			rdfObjectHandle.minWidth = 40;
			rdfObjectHandle.minHeight = 20;
			
			this.addChild(rdfObjectHandle);
			return rdfClass;
		}
		 	
		public function getRdfNode(rdfName:String): RDFNode {			
			var index:int = searchClassById(rdfName, _rdfNodes);
			if( index >= 0 ) {
				return _rdfNodes[index];														
			}
			
			return null;
		}
		
		public function getRdfArc(rdfName:String): RDFArc {			
			var index:int = searchArcByPropertyId(rdfName, _arcs);
			if( index >= 0 ) {
				return _arcs[index];														
			}
			
			return null;
		}
		
		public function loadVocabulary(url:String = null):void
		{
			if( url ) {
				this._vocabularyURL = url;	
			}
			
			if( this._vocabularyURL ) {
				var urlr:URLRequest = new URLRequest(this._vocabularyURL);
				_loader = new URLLoader(urlr);
				_loader.addEventListener(Event.COMPLETE, onVocabularyLoadHandler);
			}
		}
		
		private function loadVocabularyLayout(url:String = null) : void
		{
			if( url ) {
				this._layoutURL = url;	
			}
			
			if( this._layoutURL ) {
				var urlr:URLRequest = new URLRequest(this._layoutURL);
				_loader = new URLLoader(urlr);
				_loader.addEventListener(Event.COMPLETE, onVocabularyLayoutLoadHandler);
			}
		}
		
		private function searchClassById(id:String, classesArray:Array): int {
			for( var i:uint = 0; i < classesArray.length; i++ ) {
				if( (classesArray[i] as RDFNode).rdfName == id ) {
					return i;
				}
			}
			return -1;
		}
		
		private function searchArcByPropertyId(id:String, arcsArray:Array): int {
			for( var i:uint = 0; i < arcsArray.length; i++ ) {
				if( (arcsArray[i] as RDFArc).RDFid == id ) {
					return i;
				}
			}
			return -1;
		}
		
		/**
		 * This callback is called when....  
		 * 
		 */
		private function onVocabularyLoadHandler(event:Event) : void
		{
			var vocabulary:XML = XML(_loader.data);
			// initialize class attribute _arcs where all the arcs are stored
			_arcs = new Array();
			_rdfNodes = new Array();
			// parse XML process
			// hold all the classes
			var elementName:String;
			var rdfClassId:String;
			var vocabularyPrefix:String = new String(vocabulary.@id.toString());
			for each( var rdfClass:XML in vocabulary.classes.Class ) {
				rdfClassId = vocabularyPrefix + ":" + rdfClass.@id;
				var newClass:Object = new Object(); //_classes[rdfClassId] = new Object();
				newClass.id = vocabularyPrefix + ":" + rdfClass.@id;
				
				for each( var classElement:XML in rdfClass.children() ) {
					elementName = classElement.name().localName;
					
					if( elementName == "label" ) {
						newClass.label = rdfClass.label.toString();
					} 
					else if( elementName == "comment" ) {
						newClass.comment = rdfClass.comment.toString();
					}
					else if( elementName == "subClassOf" ) {
						if( newClass.subClassOf == undefined ) {
							newClass.subClassOf = new Array();
						} 
						(newClass.subClassOf as Array).push(rdfClass.subClassOf[ newClass.subClassOf.length ].@resource.toString());
					}
					else if( elementName == "disjointWith" ) {
						if( newClass.disjointWith == undefined ) {
							newClass.disjointWith = new Array();
						} 
						(newClass.disjointWith as Array).push(rdfClass.disjointWith[ newClass.disjointWith.length ].@resource.toString());
					}
				}
				
				_classes.push(newClass);
            }
			
			// get properties
			var rdfPropertyId:String;
			for each( var rdfProperty:XML in vocabulary.properties.Property ) {
				var newProperty:Object = new Object();
				newProperty.id = vocabularyPrefix + ":" + rdfProperty.@id.toString();
				
				for each( var propertyElement:XML in rdfProperty.children() ) {
					elementName = propertyElement.name().localName;
					
					if( elementName == "label" ) {
						newProperty.label = rdfProperty.label.toString();
					} 
					else if( elementName == "comment" ) {
						newProperty.comment = rdfProperty.comment.toString();	
					}
					else if( elementName == "domain" ) {
						if( newProperty.domain == undefined ) {
							newProperty.domain = new Array();
						} 
						(newProperty.domain as Array).push(rdfProperty.domain[ newProperty.domain.length ].@resource.toString());
					}
					else if( elementName == "range" ) {
						if( newProperty.range == undefined ) {
							newProperty.range = new Array();
						} 
						(newProperty.range as Array).push(rdfProperty.range[ newProperty.range.length ].@resource.toString());
					}
					else if( elementName == "subPropertyOf " ) {
						if( newProperty.subPropertyOf  == undefined ) {
							newProperty.subPropertyOf  = new Array();
						} 
						(newProperty.subPropertyOf as Array).push(rdfProperty.subPropertyOf[ newProperty.subPropertyOf.length ].@resource.toString());
					}
				}
				
				_properties.push(newProperty);
			}
            
            // comment by guidocecilio - 10 July 2009
            // create arcs between each class and superclass
			var rdfArc:RDFArc = null;
			var rdfNodeSuperClass:RDFNode;
			var rdfNodeClass:RDFNode;
			var id:String;
			var index:int;
			for each( var classIterator:Object in _classes ) {
				
				index = searchClassById(classIterator.id, _rdfNodes);
				if( index < 0 ) {
					rdfNodeClass = this.addRdfClass(classIterator.id, classIterator.label, false);
					_rdfNodes.push(rdfNodeClass);
				}
				else {
					rdfNodeClass = _rdfNodes[index];														
				}
				
				if( classIterator.subClassOf != undefined ) {
					for each( var subclassofIterator:Object in classIterator.subClassOf ) {
						id = subclassofIterator.toString();
						index = -1;
						index = searchClassById(id, _rdfNodes);
						if( index < 0 ) {
							rdfNodeSuperClass = this.addRdfClass(id, id, false);
							_rdfNodes.push(rdfNodeSuperClass);
						}
						else {
							rdfNodeSuperClass = _rdfNodes[index];														
						}
						
						rdfArc = new RDFArc(null, null, rdfNodeClass, rdfNodeSuperClass, RDFArc.RDFS_SUBBCLASSOF);
						this.addChildAt(rdfArc, 0);
						_arcs.push(rdfArc);
					}
				}
			}
			
			var label:RDFPropertyLabel2;
			var rdfObjectHandle:RDFPropertyLabelObjectHandles;
			var rdfDomainClass:RDFNode;
			var rdfRangeClass:RDFNode;
			for each( var propertyIterator:Object in _properties ) {
				if( propertyIterator.domain != undefined ) {
					for each( var domainIterator:Object in propertyIterator.domain ) {
						id = domainIterator.toString();
						index = -1;
						index = searchClassById(id, _rdfNodes);
						if( index < 0 ) {
							rdfDomainClass = this.addRdfClass(id, id, false);
							_rdfNodes.push(rdfDomainClass);
						}
						else {
							rdfDomainClass = _rdfNodes[index];														
						}
						
						if( propertyIterator.range != undefined ) {
							for each( var rangeIterator:Object in propertyIterator.range ) {
								id = rangeIterator.toString();
								index = -1;
								index = searchClassById(id, _rdfNodes);
								if( index < 0 ) {
									rdfRangeClass = this.addRdfClass(id, id, false);
									_rdfNodes.push(rdfRangeClass);
								}
								else {
									rdfRangeClass = _rdfNodes[index];														
								}
								
								label = new RDFPropertyLabel2(propertyIterator.label);
								
								rdfArc = new RDFArc(propertyIterator.id, label, rdfDomainClass, rdfRangeClass, RDFArc.RDFS_PROPERTY);
								
								// add component to the container
								this.addChildAt(rdfArc, 0);
								_arcs.push(rdfArc);
								
								rdfObjectHandle = new RDFPropertyLabelObjectHandles();
								rdfObjectHandle.allowRotate = false;
								rdfObjectHandle.mouseCursors = null;//new ObjectHandlesMouseCursors2();
								rdfObjectHandle.addChild(label);
								rdfArc.objectHandler = rdfObjectHandle;
								rdfObjectHandle.width = 100;
								rdfObjectHandle.height = 28;
								rdfObjectHandle.minWidth = 40;
								rdfObjectHandle.minHeight = 20;
								
								this.addChildAt(rdfObjectHandle, 0);
								this.setChildIndex(rdfObjectHandle, _arcs.length);
							}
						}
					}
				}
			}
			
			// this is wrong we need to fire event instead doing this
			showArcProperties( (this.parentApplication as Diagram).showProperties );
            // chain load layout
            this.loadVocabularyLayout();
		}
		
		/**
		 * Show or hide the label of all arcs
		 */ 
		public function showArcProperties(value:Boolean):void {
			for each (var arc:RDFArc in _arcs) {
				 arc.showLabel(value);
			}
		}
		
		private function onVocabularyLayoutLoadHandler(event:Event) : void
		{
			var layout:XML = XML(_loader.data);
			var node:XML;
			var rdfHandle:RDFNode;
			
			// old version
			if( !layout.child("nodes").length() ) {
				for each (node in layout.children()) {
					rdfHandle = this.getRdfNode(node.@name.toString());
					if( rdfHandle != null ) {
						rdfHandle.objectHandle.x = Number(node.@x);
						rdfHandle.objectHandle.y = Number(node.@y);
						rdfHandle.objectHandle.width = Number(node.@width);
						rdfHandle.objectHandle.height = Number(node.@height);
					}
				}
			}
			// new version
			else {
				for each (node in layout.nodes.children()) {
	            	rdfHandle = this.getRdfNode(node.@name.toString());
					if( rdfHandle != null ) {
						rdfHandle.objectHandle.x = Number(node.@x);
						rdfHandle.objectHandle.y = Number(node.@y);
						rdfHandle.objectHandle.width = Number(node.@width);
						rdfHandle.objectHandle.height = Number(node.@height);
	            	}
	            }
				
				// if there is some arc that needs configuration, this is the correct place to setting it
				for each (node in layout.arcs.children()) {
					var rdfArc:RDFArc = this.getRdfArc( node.@id.toString() );
					if( rdfArc != null ) {
						if( node.child("controlPoint").length() ) {
							rdfArc.controlPoint = new Point(node.controlPoint.@x, node.controlPoint.@y);
						}
						
						if( node.label.@linked.toString() == "false" ) {
							rdfArc.labelManuallyMoved = true;
							rdfArc.objectHandler.x = Number(node.label.@x);
							rdfArc.objectHandler.y = Number(node.label.@y);
							
							var width:Number = Number(node.label.@width.toString());
							var height:Number = Number(node.label.@height.toString());
							rdfArc.objectHandler.width = width == 0 ? 100 : width;
							rdfArc.objectHandler.height = height == 0 ? 28 : height;
						}
					}
				}
			}

			this.dispatchEvent( new Event(VocabularyCanvas.VOCABULARY_READY, true));
		}
		
		public function get vocabularyLayout():XML
		{
			var layout:XML = <layout></layout>;
			var nodes:XML = <nodes></nodes>;
			var arcs:XML = <arcs></arcs>;
			
			for each(var handler:Object in this.getChildren()) {
				
				if( (handler is RDFObjectHandles) && !(handler is RDFPropertyLabelObjectHandles) ) {
					//trace(RDFObjectHandles(handler).rdfNode);
					//trace(RDFObjectHandles(handler).rdfNode.rdfName);
					var node:XML = <node type="class" name={RDFObjectHandles(handler).rdfNode.rdfName} 
										x={RDFObjectHandles(handler).x}
										y={RDFObjectHandles(handler).y}
										width={RDFObjectHandles(handler).width}
										height={RDFObjectHandles(handler).height}
									/>;
					nodes.appendChild(node);
				}
				else if( handler is RDFArc ) {
					var rdfarc:RDFArc = (handler as RDFArc);
					if( rdfarc.controlPoint || rdfarc.labelManuallyMoved ) {
						var arc:XML = <arc id={rdfarc.RDFid}></arc>;
						arc.appendChild(XML(<label text={rdfarc.label.text} linked={!rdfarc.labelManuallyMoved} x={rdfarc.objectHandler.x} y={rdfarc.objectHandler.y} 
							width={rdfarc.objectHandler.width} height={rdfarc.objectHandler.height}/> ) );

						if( rdfarc.controlPoint ) {
							arc.appendChild(XML(<controlPoint x={rdfarc.controlPoint.x} y={rdfarc.controlPoint.y}/> ) );	
						}
						
						arcs.appendChild(arc);
					}
				}
			}
			
			layout.appendChild(nodes);
			layout.appendChild(arcs);
			
			return layout;
		}
	
		public function get vocabularyBitmap():BitmapData {
			var bd:BitmapData = new BitmapData(this.width, this.height, true, 0xffffff);
			bd.draw(this);
			return bd;
		}
		
		public function get renderNodesShadow():Boolean {
			return _renderNodesShadow;
		}
		
		public function set renderNodesShadow(value:Boolean):void {
			_renderNodesShadow = value;
			for each(var handler:Object in this.getChildren()) {
				if( (handler is RDFObjectHandles) && !(handler is RDFPropertyLabelObjectHandles) ) {
					RDFObjectHandles(handler).rdfNode.renderShadow = value;	
				}
				else if( handler is RDFPropertyLabelObjectHandles ) {
					RDFPropertyLabel2(RDFObjectHandles(handler).getChildAt(0)).renderShadow = value;	
				}
			}	
		}
		
		public function set layoutURL(value:String):void {
			this._layoutURL = value;
		}
		
		public function set vocabularyURL(value:String):void {
			this._vocabularyURL = value;	
		}
		
		public function onSelectArc(event:Event):void {
			event.stopImmediatePropagation();
			
			var newSelectedArc:RDFArc = (event.target as RDFArc); 
			if( !selectedArc ) {
				selectedArc = newSelectedArc;
			}
			else if( selectedArc != newSelectedArc ) {
				// force a deselect
				selectedArc.select(true);
				selectedArc = newSelectedArc; 
			}
		}
		
		public function onDeselectArc(event:Event):void {
			event.stopImmediatePropagation();
			selectedArc = null;
		}
		
		public function onManuallyAjustPropertyLabel(event:Event):void {

			var children:Array = (event.target as Container).getChildren();
			for( var i:uint = 0; i < children.length; i++ ) {
				var child:DisplayObject = children[i];
				if( child is RDFPropertyLabel2 ) {
					(child as RDFPropertyLabel2).RDFArcReference.labelManuallyMoved = !(child as RDFPropertyLabel2).RDFArcReference.labelManuallyMoved;	
					break;
				}
			}

			trace(event);	
		}
		
		public function get arcs():Array {
			return _arcs;
		}

	}
}