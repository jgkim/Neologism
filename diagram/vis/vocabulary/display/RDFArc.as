package vis.vocabulary.display
{
	
	import com.example.programmingas3.SpriteArranger.CircleSprite;
	import com.example.programmingas3.SpriteArranger.GeometricSprite;
	
	import flash.display.DisplayObject;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	
	import mx.core.UIComponent;
	
	public class RDFArc extends UIComponent
	{
		public static const ARC_SELECTED:String = "arc_selected";
		public static const ARC_DESELECTED:String = "arc_deselected";
		public static const LABEL_MANUALLYMOVED:String = "label_was_moved_manually";
		public static const LABEL_RESETPOSITION:String = "label_reset_position";
		
		
		//private static var PROPERTY_STYLE:Array = [2, 0x73d216, 1];
		//private static var SUBCLASS_STYLE:Array = [2, 0xcc0000, 1];
		private static var PROPERTY_STYLE:Array = [2, 0x11277e, 1];
		private static var PROPERTY_STYLE_SELECTED:Array = [2, 0x73d216, 1];
		private static var SUBCLASS_STYLE:Array = [2, 0x554600, 1];
		private var arrowProperties:Array;

		
		public static var RDFS_SUBBCLASSOF:String = 'rdfs:subClassOf';
		public static var RDFS_PROPERTY:String = 'rdfs';
		
		//referece to
		private var _id:String;
		private var _label:RDFPropertyLabel2;
		private var _ohLabelWrapper:RDFPropertyLabelObjectHandles;
		
		private var _domain:RDFNode;
		private var _range:RDFNode;
		
		private var _index:Number;
		private var _type:String;
		
		private var _arcDeviation:Number;
		
		private var _p1:Point;
		private var _p2:Point;
		private var _control:Point = null;
		
		private var _initialized:Boolean = false;
				
		// new fields
		private var thicknessNumber:uint = 2;
	    private var alphaNumber:uint = 4.0;
	    private var arrowWidth:int = 4;
	    private var arrowHeight:int = 8;
	
	    private var color:uint = 0x000000;
	//  this box referance for find line distance
	    //private var fromBox:Box;
	    //private var toBox:Box;
	//  this boolean variable understand selection on,off    
	    private var isSelect:Boolean = false;
		private var isControlPointSelected:Boolean = false;
		
		private var controlPointGS:GeometricSprite;
		// this attribute is for autocalculate the _controlp
		private var autoValue:Boolean = true;
		// this private attr is to hold the arc deepth when this is selected for edition and is swaped 
		// to a higher deepth 
		private var holderIndex:int = -1;
		
		private var _labelManuallyMoved:Boolean = false;
		private var _editMode:Boolean = false;
		 
		private var _ajustingDiameter:Boolean = false;
		private var _diameter:Number = 30;
		private var _startX:Number;
		private var _startY:Number;
		
		/**
		 *
		 */
		public function RDFArc(id:String, label:RDFPropertyLabel2, d:RDFNode, r:RDFNode, type:String)
		{
			super();
		
			this._label = label;
			if( this._label ) {
				this._label.RDFArcReference = this;
			}
			
			this._id = id;
			this._domain = d;
			this._range = r;
			this._index = 0;
			this._type = type;
			this._arcDeviation = 45;
		
			arrowProperties = PROPERTY_STYLE;
			
			buttonMode = true;
			if( _editMode ) {
				this.addEventListener(MouseEvent.CLICK, mouseClick);
				this.addEventListener(MouseEvent.MOUSE_OVER, onMouseOverArc);
				this.addEventListener(MouseEvent.MOUSE_OUT, onMouseOutArc);
				
				addEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, true);
				addEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, false);
			}
      	}
		
		override protected function createChildren():void {
			//super.createChildren();
			
			this.percentHeight = 100;
			this.percentWidth = 100;
			
			controlPointGS = new CircleSprite(10, 0x000000, 0xee0000);
			controlPointGS.addEventListener(MouseEvent.MOUSE_DOWN, selectControlPoint);
			controlPointGS.addEventListener(MouseEvent.MOUSE_UP, selectControlPoint);
			controlPointGS.addEventListener(MouseEvent.MOUSE_MOVE, movingControlPoint);
			controlPointGS.visible = isSelect;
			this.addChild(controlPointGS);
		}
		
		// Implement the commitProperties() method. 
		override protected function commitProperties():void {
		    super.commitProperties();
		    
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			draw();
			super.updateDisplayList(unscaledWidth, unscaledHeight);
		}
		
		/**
		 * In this case forced parameters is used for draw the arrow in different color
		 * 
		 */ 
		public function draw(forced:Boolean = false):void	{
			
			var d:RDFObjectHandles = _domain.objectHandle;
			var r:RDFObjectHandles = _range.objectHandle;
			var p1:Point = d.arcInPoint(r);
			var p2:Point = r.arcInPoint(d);
			var arrhead:Array = [];	
			
			// local coordinates
			var l_p1:Point = toLocal(p1);
			var l_p2:Point = toLocal(p2);
			var l_p:Point = null;
			
			this.graphics.clear();
			
			if (_type == RDFArc.RDFS_PROPERTY) {
				var cw:Number = 1;
				var p:Point = null;
				var dev:Number = _arcDeviation;
				
				// if domain and range are the same
				if (d == r) {
					var arr:Array = d.arcOnItself();
					p1 = arr[0];
					p2 = arr[1];
					l_p1 = toLocal(p1);
					l_p2 = toLocal(p2);
					dev = _index * _arcDeviation;
	
				} else {
					if (p1 == null || p2 == null)
						return;
					//cw = (_index % 2 == 0) ? -1 : 1;
					cw = -1;
					dev = (int(_index/2)+1) * _arcDeviation;
				}
				
				p = isSelect || _control ? _control : Utils.middlePerpendicularPoint2(p1, p2, dev, cw);
				l_p = toLocal(p);
				
				if( _ohLabelWrapper ) {
					if( d == r ) {
						if( !labelManuallyMoved ) {
							p = new Point(l_p1.x - ((d.width / 2) + 5), l_p1.y);
							p.x -= _diameter;
							p.y -= _diameter;
							_ohLabelWrapper.x = p.x - _ohLabelWrapper.width / 2;  
							_ohLabelWrapper.y = p.y - _ohLabelWrapper.height / 2;
						}
					}
					else {
						if( !labelManuallyMoved && (_control || p) ) {
							_ohLabelWrapper.x = _control ? _control.x - _ohLabelWrapper.width / 2 : p.x - _ohLabelWrapper.width / 2;
							_ohLabelWrapper.y = _control ? _control.y - _ohLabelWrapper.height / 2 : p.y - _ohLabelWrapper.height / 2;
						}
					}
				}
				
				if( !forced ) {
					if( isSelect ) {
						// draw control lines
						if( d != r) {
							this.graphics.lineStyle(1, 0xcccccc, 1.0);
							this.graphics.moveTo(l_p1.x, l_p1.y);
							this.graphics.lineTo(l_p.x, l_p.y);
							this.graphics.moveTo(l_p2.x, l_p2.y);
							this.graphics.lineTo(l_p.x, l_p.y);
							
						}
						// select line style to draw the arc
						arrowProperties = PROPERTY_STYLE_SELECTED; 
					} 
					else {
						// normal line style for draw the arc
						arrowProperties = PROPERTY_STYLE;
					}
				}
					
				// draw zone
				this.graphics.lineStyle(arrowProperties[0], arrowProperties[1], arrowProperties[2]);
				if( d == r ) {
					// Draw an arc with a center of (250, 250) and a radius of 200
					// that starts at an angle of 45 degrees then rotates counter-
					// clockwise 90 degrees.  We'll span the arc with 20 evenly spaced points.
					//drawArc(250, 250, 200, 45/360, -90/360, 20);
					l_p1.x -= (d.width / 2) + 5;
					drawArc(l_p1.x, l_p1.y, _diameter, 1/360, -270/360, 30);
					l_p2.x = l_p1.x;
					l_p2.y = l_p1.y + _diameter;
					
					l_p1.x = l_p2.x-14;
					l_p1.y = l_p2.y-2; 

					arrhead = Utils.arrowHeads(l_p1, l_p2);
				}
				else {
					this.graphics.moveTo(l_p1.x, l_p1.y);
					this.graphics.curveTo(l_p.x, l_p.y, l_p2.x, l_p2.y);
					arrhead = Utils.arrowHeads(l_p2, l_p);
				}
			
				_initialized = true;
				
			} 
			else if (_type == RDFArc.RDFS_SUBBCLASSOF) {
				if (p1 == null || p2 == null)
					return;
					
				this.graphics.lineStyle(thicknessNumber/* SUBCLASS_STYLE[0]*/, SUBCLASS_STYLE[1], SUBCLASS_STYLE[2]);
				this.graphics.moveTo(l_p1.x,l_p1.y);
				this.graphics.lineTo(l_p2.x,l_p2.y);
				
				arrhead = Utils.arrowHeads(l_p2,l_p1);
			}
			
			// arrow head
			this.graphics.moveTo(arrhead[0].x, arrhead[0].y);
			this.graphics.lineTo(l_p2.x,l_p2.y);
			this.graphics.lineTo(arrhead[1].x, arrhead[1].y);

		}
		
		private function toLocal(p:Point) : Point 
		{
			if (p == null)
				return p;
			return contentToLocal(p);
		}
		
		public function get RDFid(): String {
			return _id;
		}
		
		public function get type() : String {
			return _type;
		}
		
		public function get domain(): RDFNode {
			return _domain;
		}
		
		public function get range(): RDFNode {
			return _range;
		}
		
		public function get label(): RDFPropertyLabel2 {
			return _label;
		}
		
		public function set label(value:RDFPropertyLabel2):void  {
			_label = value;
		}
		
		public function set objectHandler(oh:RDFPropertyLabelObjectHandles):void {
			_ohLabelWrapper = oh;
			_ohLabelWrapper.enableSelection = _editMode;
		}
		
		public function get objectHandler():RDFPropertyLabelObjectHandles {
			return _ohLabelWrapper;
		}
		
		public function get index() : Number {
			return _index;
		}
		
		public function set index(idx:Number) : void {
			_index = idx;
		}
		
		public function createArrow():void {
		}
		
		public function showLabel(value:Boolean):void {
			if( _ohLabelWrapper )
				_ohLabelWrapper.visible = value;
		}
	 
		public function mouseClick(event:MouseEvent): void {
			trace(event.target);
			if( event.target is RDFArc && !_ajustingDiameter ) {
				
				if( event.ctrlKey ){
					_control = null;
					draw();
				}
				else {
					// notify to daddy about the selection, lol
					if( isSelect ) {
						// add the mouse over and out for the arrow
						this.addEventListener(MouseEvent.MOUSE_OVER, onMouseOverArc);
						this.addEventListener(MouseEvent.MOUSE_OUT, onMouseOutArc);
						
						// notify to daddy
						event.target.dispatchEvent( new Event(RDFArc.ARC_DESELECTED, true));
					} 
					else {
						// remove the mouse over and out for the arrow
						this.removeEventListener(MouseEvent.MOUSE_OVER, onMouseOverArc);
						this.removeEventListener(MouseEvent.MOUSE_OUT, onMouseOutArc);
						// notify to daddy
						event.target.dispatchEvent( new Event(RDFArc.ARC_SELECTED, true));
					}
					
					var arc:DisplayObject = (event.currentTarget as DisplayObject);
					if( holderIndex == -1 ) {
						holderIndex = parent.getChildIndex(arc);
						parent.setChildIndex( arc, parent.numChildren-1);	
					}
					else {
						parent.setChildIndex( arc, holderIndex);
						holderIndex = -1;
					}
					
					select();
				}
			}
		}	
		
		public function select(forced:Boolean = false):void{
			var p1:Point;
			var p2:Point;
			var cw:Number = 1;
			var dev:Number = _arcDeviation;

			if( forced ) {
				parent.setChildIndex( this, holderIndex);
				holderIndex = -1;
			}
			
			if( !this.isSelect ) {
				this.isSelect = true;
				showLabel(false);
				var d:RDFObjectHandles = _domain.objectHandle;
				var r:RDFObjectHandles = _range.objectHandle;
				
				if( !_control ) {
					p1 = d.arcInPoint(r);
					p2 = r.arcInPoint(d);
					if( p1 != null && p2 != null ) {
						cw = (_index % 2 == 0) ? -1 : 1;
						dev = (int(_index/2)+1) * _arcDeviation;
						_control = Utils.middlePerpendicularPoint2(p1, p2, dev, cw);
					}

					controlPointGS.x = _control.x - (controlPointGS.width / 2);
					controlPointGS.y = _control.y - (controlPointGS.height / 2);
				}
				controlPointGS.visible = true;
			}
			else{
				//this.color = 0x000000;
				this.isSelect = false;
				
				// before show the label we need to be sure that the show labels option is enabled or disabled
				var pa:Diagram = (this.parentApplication as Diagram);
				showLabel(pa.showProperties);

				controlPointGS.visible = false;
			}
			draw();
		}
		
		public function selectControlPoint(event:MouseEvent):void {
			trace("selectControlPoint");
			if( isSelect )
			{
				isControlPointSelected = !isControlPointSelected;
			}
		}
		public function movingControlPoint(event:MouseEvent):void {
			trace("movingControlPoint");
			//trace("catching event - (" + _controlPoint.x + ", " + _controlPoint.y + ")");
			if( isControlPointSelected )
			{
				_control.x = controlPointGS.x + (controlPointGS.width / 2);
				_control.y = controlPointGS.y + (controlPointGS.height / 2);
				draw();
			}
		}
		
		public function onMouseOverArc(event:MouseEvent):void {
			arrowProperties = PROPERTY_STYLE_SELECTED;
			draw(true);
		}
		
		public function onMouseOutArc(event:MouseEvent):void {
			arrowProperties = PROPERTY_STYLE;
			draw(true);
		}
		
		public function onManuallyAjustPropertyLabel(event:Event):void {
			_labelManuallyMoved = true;	
		}

		public function get labelManuallyMoved(): Boolean {
			return this._labelManuallyMoved;
		}
		
		public function set labelManuallyMoved(value:Boolean): void {
			this._labelManuallyMoved = value;
		}

		public function get controlPoint(): Point {
			return this._control;
		}
		
		public function set controlPoint(p:Point): void {
			_control = p;
			controlPointGS.x = _control.x - (controlPointGS.width / 2);
			controlPointGS.y = _control.y - (controlPointGS.height / 2);
		}

		public function get editMode():Boolean {
			return _editMode;
		}
		
		public function set editMode(value:Boolean):void {
			if( _type == RDFS_PROPERTY ) {
				_editMode = value;
				_ohLabelWrapper.enableSelection = _editMode;
				if( _editMode ) {
					this.addEventListener(MouseEvent.CLICK, mouseClick);
					this.addEventListener(MouseEvent.MOUSE_OVER, onMouseOverArc);
					this.addEventListener(MouseEvent.MOUSE_OUT, onMouseOutArc);
					
					
					addEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, true);
					addEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, false);
				}
				else {
					this.removeEventListener(MouseEvent.CLICK, mouseClick);
					this.removeEventListener(MouseEvent.MOUSE_OVER, onMouseOverArc);
					this.removeEventListener(MouseEvent.MOUSE_OUT, onMouseOutArc);
					
					removeEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, true);
					removeEventListener(RDFArc.LABEL_MANUALLYMOVED, this.onManuallyAjustPropertyLabel, false);
				}
			}
		}
		
		//
		// Angles are expressed as a number between 0 and 1.  .25 = 90 degrees.
		// If you prefer using degrees, write 90 degrees like so "90/360".
		private function drawArc(centerX:Number, centerY:Number, radius:Number, startAngle:Number, arcAngle:Number, steps:Number):void {
			//
			// For convenience, store the number of radians in a full circle.
			var twoPI:Number = 2 * Math.PI;
			//
			// To determine the size of the angle between each point on the
			// arc, divide the overall angle by the total number of points.
			var angleStep:Number = arcAngle / steps;
			//
			// Determine coordinates of first point using basic circle math.
			var xx:Number = centerX + Math.cos(startAngle * twoPI) * radius;
			var yy:Number = centerY + Math.sin(startAngle * twoPI) * radius;
			//
			// Move to the first point.
			this.graphics.moveTo(xx, yy);
			//
			// Draw a line to each point on the arc.
			for(var i:Number = 1; i <= steps; i++){
				//
				// Increment the angle by "angleStep".
				var angle:Number = startAngle + i * angleStep;
				//
				// Determine next point's coordinates using basic circle math.
				xx = centerX + Math.cos(angle * twoPI) * radius;
				yy = centerY + Math.sin(angle * twoPI) * radius;
				//
				// Draw a line to the next point.
				this.graphics.lineTo(xx, yy);
			}
		}
		
		
	}
}