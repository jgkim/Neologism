package vis.vocabulary.display
{
	import caurina.transitions.Tweener;
	
	import com.roguedevelopment.objecthandles.ObjectHandlesMouseCursors2;
	
	import flash.display.BitmapData;
	import flash.display.GradientType;
	import flash.display.SpreadMethod;
	import flash.events.Event;
	import flash.geom.Point;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	
	import mx.containers.Canvas;
	
	import vis.vocabulary.RDFProperty;
	
	public class VocabularyCanvas extends Canvas
	{		
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
															
		private var _drawGrid:Boolean;
		private var _gridSpaceing:Number;
		private var _loader:URLLoader;
		
		private var _vocabularyURL:String;
		private var _layoutURL:String
		
		private var _arcDeviation:Number;
		
		private var _properties:Object;
		private var _classes:Object;
		private var _arcs:Array;
		
		
		public function VocabularyCanvas()
		{
			super();
			this._drawGrid =true;
			this._gridSpaceing = 17;
			this._arcDeviation = 45;
			this._properties = {};
			this._classes = {};
			this._arcs = [];
		}
		
		public function get drawGrid() : Boolean
		{
			return this._drawGrid;
		}
		
		public function set drawGrid(dg:Boolean) : void
		{
			this._drawGrid = dg;
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
		
		public function get arcDeviation() : Number
		{
			return this._arcDeviation;
		}
		
		public function set arcDeviation(ad:Number) : void
		{
			this._arcDeviation = ad;
		}
		
		private function paintGrid() : void
		{
			this.graphics.beginFill(0xffffff, 1);
			this.graphics.drawRect(0, 0, this.width, this.height);
			this.graphics.endFill();
			
			this.graphics.lineStyle(GRID_STYLE[0], GRID_STYLE[1], GRID_STYLE[2]);
			
			var startGridX:Number = screen.x + _gridSpaceing - screen.x % _gridSpaceing;
			var startGridY:Number = screen.y + _gridSpaceing - screen.y % _gridSpaceing;
			for (var i:Number=startGridX; i <= screen.x + screen.width; i+= _gridSpaceing) {
				this.graphics.moveTo(i,screen.y);
				this.graphics.lineTo(i,screen.y + screen.height);
			}
			
			for (i = startGridY; i<= screen.y + screen.height; i+= _gridSpaceing) {
				this.graphics.moveTo(screen.x,i);
				this.graphics.lineTo(screen.x + screen.width,i);
			}

		}
		
		protected function drawArcs() : void
		{
			for each (var arc:RDFArc in _arcs) {
				drawArc(arc);
			}
		}
		
		private function toLocal(p:Point) : Point 
		{
			if (p == null)
				return p;
			return contentToLocal(p);
		}
		
		private function drawArc(arc:RDFArc) : void 
		{
			/*
			for each (var arc:RDFArc in _arcs) {
				if( arc.domain != null || arc.range == null )
				{
					trace("arc.domain.rdfName = " + arc.domain.rdfName + " and arc.range = null");
				}			
				if( arc.domain == null || arc.range != null )
				{
					trace("arc.domain = null and arc.range.rdfName = " + arc.range.rdfName);
				}
			}
			
			if( arc.domain != null || arc.range != null )
			{
			*/
				//trace("--------------- arc.domain.rdfName = " + arc.domain.rdfName + " and arc.range.rdfName = " + arc.range.rdfName);
				
				var d:RDFObjectHandles = arc.domain.handle;
				var r:RDFObjectHandles = arc.range.handle;
				var p1:Point = d.arcInPoint(r);
				var p2:Point = r.arcInPoint(d);
				var arrhead:Array = [];	
				
				// local coordinates
				var l_p1:Point = toLocal(p1);
				var l_p2:Point = toLocal(p2);
				var l_p:Point = null;
				
				if (arc.type == RDFArc.RDFS_PROPERTY) {
					var label:RDFPropertyLabel = arc.label;
					
					this.graphics.lineStyle(PROPERTY_STYLE[0],PROPERTY_STYLE[1],PROPERTY_STYLE[2]);
					
					var cw:Number = 1;
					var p:Point = null;
					var dev:Number = _arcDeviation;
					if (d == r) {
						var arr:Array = d.arcOnItself();
						p1 = arr[0];
						p2 = arr[1];
						l_p1 = toLocal(p1);
						l_p2 = toLocal(p2);
						dev = arc.index * _arcDeviation;
					} else {
						if (p1 == null || p2 == null)
							return;
						cw = (arc.index % 2 == 0) ? -1 : 1;
						dev = (int(arc.index/2)+1) * _arcDeviation;
					}
					
					p = Utils.middlePerpendicularPoint2(p1,p2,dev,cw);
					l_p = toLocal(p);
					
					this.graphics.moveTo(l_p1.x,l_p1.y);
					this.graphics.curveTo(l_p.x,l_p.y,l_p2.x,l_p2.y);
					
					label.x = p.x - label.width / 2;
					label.y = p.y - label.height / 2;
					
					arrhead = Utils.arrowHeads2(l_p2,l_p);
					
				} else if (arc.type == RDFArc.RDFS_SUBBCLASSOF) {
					this.graphics.lineStyle(SUBCLASS_STYLE[0],SUBCLASS_STYLE[1],SUBCLASS_STYLE[2]);
					
					if (p1 == null || p2 == null)
						return;
						
					this.graphics.moveTo(l_p1.x,l_p1.y);
					this.graphics.lineTo(l_p2.x,l_p2.y);
					
					arrhead = Utils.arrowHeads2(l_p2,l_p1);
				}
				
				// arrow head
				this.graphics.moveTo(arrhead[0].x,arrhead[0].y);
				this.graphics.lineTo(l_p2.x,l_p2.y);
				this.graphics.lineTo(arrhead[1].x,arrhead[1].y);
			//}
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void
		{
			super.updateDisplayList(unscaledWidth, unscaledHeight);
			this.graphics.clear();
			this.paintGrid();
			this.drawArcs();
		}
		
		public function addRdfClass(rdfName:String,rdfLabel:String,external:Boolean) : RDFNode
		{
			var rdfClass:RDFNode= new RDFNode(rdfName,rdfLabel,external);
			
			var rdfObjectHandle:RDFObjectHandles = new RDFObjectHandles();
			rdfObjectHandle.allowRotate = false;
			rdfObjectHandle.mouseCursors = new ObjectHandlesMouseCursors2();
			rdfObjectHandle.addChild(rdfClass);
			rdfObjectHandle.width = 100;
			rdfObjectHandle.height = 28;
			rdfObjectHandle.minWidth = 40;
			rdfObjectHandle.minHeight = 20;
			
			this.addChild(rdfObjectHandle);
			
			return rdfClass;
		}
		 	
		public function getRdfNode(rdfName:String) : RDFNode {			
			if (rdfName in _classes)
				return _classes[rdfName];
			return null;
		}
		
		public function loadVocabulary(urlVocab:String,urlLayout:String) : void
		{
			this._vocabularyURL = urlVocab;
			this._layoutURL = urlLayout;
			
			this._loadVocabulary(this._vocabularyURL);
		}
		
		protected function _loadVocabulary(url:String) : void
		{
			var _url:URLRequest = new URLRequest(url);
			_loader = new URLLoader(_url);
			_loader.addEventListener(Event.COMPLETE,onVocabularyLoadHandler);
		}
		
		protected function _loadVocabularyLayout(url:String) : void
		{
			var _url:URLRequest = new URLRequest(url);
			_loader = new URLLoader(_url);
			_loader.addEventListener(Event.COMPLETE,onVocabularyLayoutLoadHandler);
		}
		
		private function onVocabularyLoadHandler(event:Event) : void
		{
			var vocabulary:XML = XML(_loader.data);
			
			var _class_names:Array = []
			_classes = {};
			_properties = {};
			_arcs = [];
			
			var rdf_cls:String = '';
			var rdf_prop:String = '';
			var rdf_range:Array = null;
			var rdf_domain:Array = null;
			var rdf_data:String = '';
			var rdf_cd:String = '';
			var rdf_cr:String = '';
			var rdf_arc:RDFArc = null;
			var arc_bins:Object = {};
			var rdf_superClasses:Object = {};
			
			// added by guidocecilio
			var dummyClassName:String = "Dummy Class";
			var dummyClassNumber:Number = 0;
			
			trace('==== Get Classes ====');
			for each (var rdfClass:XML in vocabulary.classes.rdfclass) {
            	rdf_cls = String(rdfClass.@name);
            	_classes[rdf_cls] = this.addRdfClass(rdf_cls,rdfClass.@label,false);
            	_class_names.push(rdf_cls);
            	
            	// subclassof 
            	if (rdfClass.@subclass != undefined) {
            		rdf_superClasses[rdf_cls] = String(rdfClass.@subclass).split(',');
            	}
            	trace('CLASS : '+rdf_cls+' - '+_classes[rdf_cls]);
            }
            
            for (rdf_cd in rdf_superClasses) {
            	for each(rdf_cr in rdf_superClasses[rdf_cd]) {
    				// added by guidocecilio - 07 July 2009
    				// check if there is a real class, if doesn't then create a dummy class
    				if( _classes[rdf_cr] == null ) {
    					dummyClassNumber++;
    					
    					// where rdf_cr is the name of the class and dummyClassName + dummyClassNumber.toString() the label
    					_classes[rdf_cr] = this.addRdfClass(rdf_cr, dummyClassName + dummyClassNumber.toString(), false);
    				}
    				
    				rdf_arc = new RDFArc(null,_classes[rdf_cd],_classes[rdf_cr],RDFArc.RDFS_SUBBCLASSOF);
    				_arcs.push(rdf_arc);
            	}
    		}
            
            trace('==== Get Properties ====');
            for each (var rdfProperty:XML in vocabulary.properties.rdfproperty) {
            	rdf_prop = String(rdfProperty.@name);
            	_properties[rdf_prop] = new RDFProperty(rdf_prop,rdfProperty.@label);
            	if (rdfProperty.@domain == undefined)
            		_properties[rdf_prop].domain = _class_names;
            	else 
            		_properties[rdf_prop].domain = String(rdfProperty.@domain).split(',');
            	
            	// scan for new classes
            	for each(rdf_cls in _properties[rdf_prop].domain) {
            		if (_class_names.indexOf(rdf_cls) == -1) {
            			_classes[rdf_cls] = this.addRdfClass(rdf_cls,rdf_cls,true);
            		}  
            	}
            	
            	//comment by guidocecilio
            	// if property range == undefined 
            	if (rdfProperty.@range == undefined)
            		_properties[rdf_prop].range = _class_names;
            	else 
            		_properties[rdf_prop].range = String(rdfProperty.@range).split(',');
            	
            	// scan for new classes
            	for each(rdf_cls in _properties[rdf_prop].range) {
            		if (_class_names.indexOf(rdf_cls) == -1) {
            			_classes[rdf_cls] = this.addRdfClass(rdf_cls,rdf_cls,true);
            		}  
            	}
            	
            	trace('PROP : '+rdf_prop+' | '+_properties[rdf_prop].domain+' | '+_properties[rdf_prop].range);
            }
            
            trace('==== Init ARC Bins ====');
            for (rdf_cd in _classes) {
            	arc_bins[rdf_cd] = {}
            	for (rdf_cr in _classes) {
            		arc_bins[rdf_cd][rdf_cr] = 0;
            	}
            }
            
            trace('==== Generate ARC Labels ====');
            for each(var p:RDFProperty in _properties) {
            	for each(rdf_cd in p.domain) {
            		for each(rdf_cr in p.range) {
            			arc_bins[rdf_cd][rdf_cr] += 1;
            			rdf_arc = new RDFArc(p.rdfLabel,_classes[rdf_cd],_classes[rdf_cr],RDFArc.RDFS_PROPERTY);
            			rdf_arc.index = arc_bins[rdf_cd][rdf_cr];
            			_arcs.push(rdf_arc);
            			this.addChild(rdf_arc.label);
            		}
            	}
            }
            
            // chain load layout
            this._loadVocabularyLayout(this._layoutURL);
		}
		
		private function onVocabularyLayoutLoadHandler(event:Event) : void
		{
			var layout:XML = XML(_loader.data);
			for each (var rdfClass:XML in layout.rdfclass) {
            	var rdfHandle:RDFNode = this.getRdfNode(rdfClass.@name);
            	if (rdfHandle != null) {
	            	var _x:Number = Number(rdfClass.@x);
	            	var _y:Number = Number(rdfClass.@y);
	            	var _width:Number = Number(rdfClass.@width);
	            	var _height:Number = Number(rdfClass.@height);
	            	
	            	Tweener.addTween(rdfHandle.handle,
	            		{x:_x, y:_y, width:_width, height:_height, time:1, transition:"linear"});
            	}
            }
		}
		
		public function get vocabularyLayout() : XML
		{
			var layout:XML = 
				<layout>
				</layout>;
			for each(var handler:Object in this.getChildren()) {
				if (handler is RDFObjectHandles) {
					var node:XML = <rdfclass name={RDFObjectHandles(handler).rdfNode.rdfName} 
										x={RDFObjectHandles(handler).x}
										y={RDFObjectHandles(handler).y}
										width={RDFObjectHandles(handler).width}
										height={RDFObjectHandles(handler).height}
									/>;
					layout.appendChild(node);
				}
			}
			return layout;
		}
	
		public function get vocabularyBitmap() : BitmapData {
			var bd:BitmapData = new BitmapData(this.width,this.height,true,0xffffff);
			bd.draw(this);
			return bd;
		}
	}
}