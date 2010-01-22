package com.example.programmingas3.SpriteArranger
{
	
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.display.Sprite;
	
	//import mx.core.UIComponent;
	
	public class Arc extends Sprite//UIComponent
	{
		private static var PROPERTY_STYLE:Array = [2, 0x73d216, 1];
		private static var SUBCLASS_STYLE:Array = [2, 0xcc0000, 1];

		public static var RDFS_SUBBCLASSOF:String = 'rdfs:subClassOf';
		public static var RDFS_PROPERTY:String = 'rdfs';
		
		private var _strlabel:String;
	
		
		private var _index:Number;
		private var _type:String;
		
		private var _arcDeviation:Number;
		
		private var _p1:Point;
		private var _p2:Point;
		private var _control:Point;
		
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
		private var _controlPoint: GeometricSprite;
				
		public function Arc(p1:Point, p2:Point, control:Point)
		{
			//super();
		
			//this._strlabel = txt;
	
			this._index = 0;
			//this._type = type;
			this._arcDeviation = 45;
			//_p1 = d.arcInPoint(r);
			//_p2 = r.arcInPoint(d);
			_p1 = p1;
			_p2 = p2;
			_control = control;
			
      		this.addEventListener(MouseEvent.CLICK, mouseClick);
			_controlPoint = new CircleSprite(10);
			_controlPoint.addEventListener(MouseEvent.MOUSE_DOWN, selectControlPoint);
			_controlPoint.addEventListener(MouseEvent.MOUSE_UP, selectControlPoint);
			_controlPoint.addEventListener(MouseEvent.MOUSE_MOVE, movingControlPoint);
			this.addChild(_controlPoint);
			
			trace("constructor called");
		}
		
		public function initialize():void {
			var p1:Point = new Point();
			p1.x = (_p1.x + _control.x) / 2;
			p1.y = (_p1.y + _control.y) / 2;
			
			var p2:Point = new Point();
			p2.x = (_p2.x + _control.x) / 2;
			p2.y = (_p2.y + _control.y) / 2;
			
			
			_controlPoint.x = _control.x - (_controlPoint.width / 2);
			_controlPoint.y = _control.y - (_controlPoint.height / 2);
			//_controlPoint.x = (p1.x + p2.x) / 2;
			//_controlPoint.y = (p1.y + p2.y) / 2;
		}
		/*
		override protected function createChildren():void {
			//super.createChildren();
			
			this.percentHeight = 100;
			this.percentWidth = 100;
	
		}
		
		// Implement the commitProperties() method. 
		override protected function commitProperties():void {
		    super.commitProperties();
		    
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void {
			draw();
			super.updateDisplayList(unscaledWidth, unscaledHeight);
		}
		*/
		
		public function drawArc():void	{
				//var p1:Point = new Point();
				//p1.x = (_p1.x + _control.x) / 2;
				//p1.y = (_p1.y + _control.y) / 2;
				
				//var p2:Point = new Point();
				//p2.x = (_p2.x + _control.x) / 2;
				//p2.y = (_p2.y + _control.y) / 2;
				
				//_control.x = 2 * (2 * _controlPoint.x - p1.x) - _p2.x;
				//_control.y = 2 * (2 * _controlPoint.y - p1.y) - _p2.y;
				
				var p1:Point = new Point();
				p1.x = (_p1.x + _control.x) / 2;
				p1.y = (_p1.y + _control.y) / 2;
				
				var p2:Point = new Point();
				p2.x = (_p2.x + _control.x) / 2;
				p2.y = (_p2.y + _control.y) / 2;
				
				//_controlPoint.x = (p1.x + p2.x) / 2;
				//_controlPoint.y = (p1.y + p2.y) / 2;
				
				// _control.x = _controlPoint.x;
				 //_control.y = _controlPoint.y;
				
				this.graphics.clear();
				this.graphics.lineStyle(PROPERTY_STYLE[0],PROPERTY_STYLE[1],PROPERTY_STYLE[2]);
				this.graphics.moveTo(_p1.x, _p1.y);
				this.graphics.curveTo(_control.x, _control.y, _p2.x, _p2.y);

				this.graphics.lineStyle(1, 0xcccccc, 1.0);
				this.graphics.moveTo(_p1.x, _p1.y);
				this.graphics.lineTo(_control.x, _control.y);
				this.graphics.moveTo(_p2.x, _p2.y);
				this.graphics.lineTo(_control.x, _control.y);
				
				/*this.graphics.lineStyle(1, 0x000000, 1.0);
				this.graphics.moveTo(p1.x, p1.y);
				this.graphics.lineTo(p2.x, p2.y);*/
				
				
		}
		/*
		private function toLocal(p:Point) : Point 
		{
			if (p == null)
				return p;
			return contentToLocal(p);
		}
		
		public function get text() : String {
			if (_label != null) 
				return _label.text;
			return null;
		}
		
		public function get type() : String {
			return _type;
		}
		
		public function set text(t:String) : void {
			if (_label != null)
				_label.text = t;
		}
		
		public function get domain() : RDFNode {
			return _domain;
		}
		
		public function get range() : RDFNode {
			return _range;
		}
		
		public function get label() : RDFPropertyLabel {
			return _label;
		}
		*/
		public function get index() : Number {
			return _index;
		}
		
		public function set index(idx:Number) : void {
			_index = idx;
		}
		
		public function createArrow():void {
		}
		
	 
	    public function mouseClick(event:MouseEvent):void {
	    	select();
	    }
	//  this method select line 
	    public function select():void{
	      trace("selected");
	      if (!this.isSelect){
	        this.color = 0xff0000;
	        this.isSelect = true;
	      }
	      else{
	        this.color = 0x000000;
	        this.isSelect = false;
	      }
	      //draw();
	    }
		
		public function selectControlPoint(event:MouseEvent):void {
			trace("catching event - (" + _controlPoint.x + ", " + _controlPoint.y + ")");
			isSelect = !isSelect;
			if( !isSelect )
			{

			}
		}
		public function movingControlPoint(event:MouseEvent):void {
			trace("catching event - (" + _controlPoint.x + ", " + _controlPoint.y + ")");
			if( isSelect )
			{
				_control.x = _controlPoint.x + (_controlPoint.width / 2);
				_control.y = _controlPoint.y + (_controlPoint.height / 2);
				//_control.x = event as MouseEvent. 
				drawArc();
			}
		}
		
	 
	}
}