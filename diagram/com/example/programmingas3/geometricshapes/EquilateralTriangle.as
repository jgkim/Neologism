package com.example.programmingas3.geometricshapes 
{
	public class EquilateralTriangle extends RegularPolygon
	{
	    // Inherits the numSides and sideLength properties from RegularPolygon
	
		public function EquilateralTriangle(len:Number = 100):void
		{
			super(len, 3);
		}
		
		public override function getArea():Number
		{
		    // the formula is ((sideLength squared) * (square root of 3)) / 4
		    return ( (this.sideLength * this.sideLength) * Math.sqrt(3) ) / 4;
		}
		
		// Inherits the getPerimeter() method from RegularPolygon
		
		// Inherits the getSumOfAngles() method from RegularPolygon
		
		public override function describe():String
		{
		    // starts with the name of the shape, then delegates the rest
		    // of the description work to the RegularPolygon superclass
		    var desc:String = "This shape is an equilateral Triangle.\n";
		    desc += super.describe();
		    return desc;
		}
	}
}