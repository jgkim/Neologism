package com.example.programmingas3.SpriteArranger
{
    import com.example.programmingas3.geometricshapes.EquilateralTriangle;
    
	public class TriangleSprite extends GeometricSprite 
	{
		//public var triangle:EquilateralTriangle;
		
		public function TriangleSprite(size:Number = 100, lColor:Number = 0x000000, fColor:Number = 0xCCCCEE)
		{
			super(size, lColor, fColor);
			this.shapeType = "Triangle";
			this.geometricShape = new EquilateralTriangle(size);
			
			drawShape();
		}
		
		public override function drawShape():void
		{
			this.graphics.clear();
			
			this.graphics.lineStyle(1.0, this.lineColor, 1.0);
			this.graphics.beginFill(this.fillColor, 1.0);
			
			this.graphics.moveTo(0, this.size);
			this.graphics.lineTo((this.size / 2), 0);
			this.graphics.lineTo(this.size, this.size);
			this.graphics.lineTo(0, this.size);
		}
	}
}