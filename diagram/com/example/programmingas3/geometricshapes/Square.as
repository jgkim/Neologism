package com.example.programmingas3.geometricshapes 
{
	public class Square extends RegularPolygon 
	{
	    // Inherits the numSides and sideLength properties from RegularPolygon
	    	    
		public function Square(len:Number = 100):void
		{
			super(len, 4);
		}
		
		public override function getArea():Number
		{
		    return (this.sideLength * this.sideLength);
		}
		
		// Inherits the getPerimeter() method from RegularPolygon
		
		// Inherits the getSumOfAngles() method from RegularPolygon
		
		public override function describe():String
		{
		    // starts with the name of the shape, then delegates the rest
		    // of the description work to the RegularPolygon superclass
		    var desc:String = "This shape is a Square.\n";
		    desc += super.describe();
		    return desc;
		}
	}
}