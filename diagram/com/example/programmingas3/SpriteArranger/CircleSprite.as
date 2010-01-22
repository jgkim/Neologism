package com.example.programmingas3.SpriteArranger
{
    import com.example.programmingas3.geometricshapes.Circle;
	import flash.display.Sprite;
    
	public class CircleSprite extends GeometricSprite 
	{
		public function CircleSprite(diam:Number = 100, lColor:Number = 0x000000, fColor:Number = 0xEECCCC)
		{
			super(diam, lColor, fColor);
			
			this.shapeType = "Circle";
			this.geometricShape = new Circle(diam);
			
			drawShape();
		}
		
		public override function drawShape():void
		{
			this.graphics.clear();
			
			this.graphics.lineStyle(1.0, this.lineColor, 1.0);
			this.graphics.beginFill(this.fillColor, 1.0);
			
			var radius:Number = this.size / 2;
			this.graphics.drawCircle(radius, radius, radius);
		}
	}
}