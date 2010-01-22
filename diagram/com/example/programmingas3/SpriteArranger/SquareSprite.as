package com.example.programmingas3.SpriteArranger
{
    import com.example.programmingas3.geometricshapes.Square;
    import flash.events.MouseEvent;
    
	public class SquareSprite extends GeometricSprite
	{
		public function SquareSprite(size:Number = 100, lColor:Number = 0x000000, fColor:Number = 0xCCEECC)
		{
			super(size, lColor, fColor);
			this.shapeType = "Square";
			this.geometricShape = new Square(size);
			
			drawShape();
		}
		
		public override function drawShape():void
		{
			this.graphics.clear();
			
			this.graphics.lineStyle(1.0, this.lineColor, 1.0);
			this.graphics.beginFill(this.fillColor, 1.0);
			
			this.graphics.drawRect(0, 0, this.size, this.size);
		}
	}
}