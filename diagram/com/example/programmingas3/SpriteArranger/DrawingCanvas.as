package com.example.programmingas3.SpriteArranger
{
	import flash.display.Sprite;
	import flash.display.DisplayObject;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	public class DrawingCanvas extends Sprite 
	{
		public var bounds:Rectangle;
		public var lineColor:Number;
		public var fillColor:Number;
		
		public function DrawingCanvas(w:Number = 500, h:Number = 200, fillColor:Number = 0xFFFFFF, lineColor:Number = 0x000000)
		{
			super();
			this.bounds = new Rectangle(0, 0, w, h);
			
			this.addEventListener(MouseEvent.MOUSE_UP, onMouseUp);
		}
		
		public function initCanvas(fillColor:Number = 0xFFFFFF, lineColor:Number = 0x000000):void
		{
			this.lineColor = lineColor;
			this.fillColor = fillColor;
			drawBounds();
		}
		
		public function drawBounds():void
		{
			var w:Number = 500;
			var h:Number = 200;
			
			this.graphics.clear();
			
			this.graphics.lineStyle(1.0, this.lineColor, 1.0);
			this.graphics.beginFill(this.fillColor, 1.0);
			this.graphics.drawRect(-1, 1, w + 2, h + 2);
			this.graphics.endFill();
		}
		
	    public function addShape(shapeName:String, len:Number):void
	    {
	        var newShape:GeometricSprite;
	        
	        switch (shapeName)
	        {       
                case "Triangle":
                    newShape = new TriangleSprite(len);
                    break;
                    
                case "Square":
                    newShape = new SquareSprite(len);
                    break;
                    
	            case "Circle":
                    newShape = new CircleSprite(len);
                    break;
            }
            // makes the shapes slightly transparent, so you can see what's behind them
            newShape.alpha = 0.8;
            
            this.addChild(newShape);
	    }
	    
		public function describeChildren():String
		{   
		    var desc:String = "";
		    var child:DisplayObject;
		    for (var i:int=0; i < this.numChildren; i++)
		    {
		        child = this.getChildAt(i);
		        desc += i + ": " + child + '\n';
		    }
		    return desc;
		}

		public function moveToBack(shape:GeometricSprite):void
		{
		    var index:int = this.getChildIndex(shape);
		    if (index > 0)
		    {
		        this.setChildIndex(shape, 0);
		    }
		}
		
		public function moveDown(shape:GeometricSprite):void
		{
		    var index:int = this.getChildIndex(shape);
		    if (index > 0)
		    {
		        this.setChildIndex(shape, index - 1);
		    }
		}
		
		public function moveToFront(shape:GeometricSprite):void
		{
		    var index:int = this.getChildIndex(shape);
		    if (index != -1 && index < (this.numChildren - 1))
		    {
		        this.setChildIndex(shape, this.numChildren - 1);
		    }
		}

		public function moveUp(shape:GeometricSprite):void
		{
		    var index:int = this.getChildIndex(shape);
		    if (index != -1 && index < (this.numChildren - 1))
		    {
		        this.setChildIndex(shape, index + 1);
		    }
		}
		
		/**
		 * Traps all mouseUp events and sends them to the selected shape.
		 * Useful when you release the mouse while the selected shape is
		 * underneath another one (which prevents the selected shape from
		 * receiving the mouseUp event).
		 */
		public function onMouseUp(evt:MouseEvent):void 
		{
		    var selectedSprite:GeometricSprite = GeometricSprite.selectedSprite;
		    if (selectedSprite != null && selectedSprite.isSelected())
		    {
			    selectedSprite.onMouseUp(evt);
			}
		}

	}
}