package com.example.programmingas3.SpriteArranger 
{
	import com.example.programmingas3.geometricshapes.IGeometricShape;
	
	import flash.display.Shape;
	import flash.display.Sprite;
	import flash.events.MouseEvent;
	import flash.geom.Rectangle;
    
	public class GeometricSprite extends Sprite 
	{
	    public var size:Number;
		public var lineColor:Number = 0x000000;
		public var fillColor:Number = 0xDDDDEE;
		
		public var shapeType:String = "GeometricSprite";
		
		/**
		 * An instance of a purely geometric shape, that is, one that defines
		 * a shape mathematically but not visually.
		 */
		public var geometricShape:IGeometricShape;
		
		/**
		 * Keeps track of the currently selected shape.
		 * This is a static property, so there can only be one GeometricSprite
		 * selected at any given time.
		 */
		public static var selectedSprite:GeometricSprite;
		
		/**
		 * Holds a border rectangle that is shown when this GeometricSprite instance is selected.
		 */
		public var selectionIndicator:Shape;
		
		public function GeometricSprite(size:Number = 100, lColor:Number = 0x000000, fColor:Number = 0xDDDDEE)
		{
		    this.size = size;
			this.lineColor = lColor;
			this.fillColor = fColor;
			this.addEventListener(MouseEvent.MOUSE_DOWN, onMouseDown);
			this.addEventListener(MouseEvent.MOUSE_UP, onMouseUp);
		}
		
		public function drawShape():void
		{
            // to be overridden in subclasses
		}
		
		private function onMouseDown(evt:MouseEvent):void 
		{
			this.showSelected();
			
			// limits dragging to the area inside the canvas
			var boundsRect:Rectangle = stage.getRect(stage);//this.parent.getRect(this.parent);
			boundsRect.width -= this.size;
			boundsRect.height -= this.size;
			this.startDrag(false, boundsRect);
		}
		
		public function onMouseUp(evt:MouseEvent):void 
		{
			this.stopDrag();
			hideSelected();
		}
		
		private function showSelected():void
		{
		    if (this.selectionIndicator == null)
		    {
		        // draws a red rectangle around the selected shape
		        this.selectionIndicator = new Shape();
		        this.selectionIndicator.graphics.lineStyle(1.0, 0xFF0000, 1.0);
			    this.selectionIndicator.graphics.drawRect(-1, -1, this.size + 2, this.size + 1);
			    this.addChild(this.selectionIndicator);
		    }
		    else
		    {
		        this.selectionIndicator.visible = true;
		    }
		    
		    if (GeometricSprite.selectedSprite != this)
		    {
    		    if (GeometricSprite.selectedSprite != null)
    		    {
    		        GeometricSprite.selectedSprite.hideSelected();
    		    }
		        GeometricSprite.selectedSprite = this;
		    }
		}
		
		private function hideSelected():void
		{
		    if (this.selectionIndicator != null)
		    {		    
		        this.selectionIndicator.visible = false;
		    }
		}
		
		/**
		 * Returns true if this shape's selection rectangle is currently showing.
		 */
		public function isSelected():Boolean
		{
		    return !(this.selectionIndicator == null || this.selectionIndicator.visible == false);
		}
		
		
		public override function toString():String
		{
		    return this.shapeType + " of size " + this.size + " at " + this.x + ", " + this.y;
		}
	}
}