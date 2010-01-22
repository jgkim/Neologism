package com.example.programmingas3.geometricshapes 
{
    /**
     * A regular polygon is equilateral (all sides are the same length)
     * and equiangular (all interior angles are the same).
     */ 
	public class RegularPolygon implements IPolygon 
	{ 
	    public var numSides:int;
		public var sideLength:Number;
		
		public function RegularPolygon(len:Number = 100, sides:int = 3):void
		{
			this.sideLength = len;
			this.numSides = sides;
		}
		
		public function getArea():Number
		{
		    // this method should be overridden in subclasses
		    return 0;
		}
		
		public function getPerimeter():Number
		{
		    return sideLength * numSides;
		}
		
		public function getSumOfAngles():Number
		{
		    if (numSides >= 3)
		    {
		        return ((numSides - 2) * 180);
		    }
		    else
		    {
		        return 0;
		    }
		}
		
		public function describe():String
		{
		    var desc:String = "Each side is " + sideLength + " pixels long.\n";
            desc += "Its area is " + getArea() + " pixels square.\n";
            desc += "Its perimeter is " + getPerimeter() + " pixels long.\n"; 
            desc += "The sum of all interior angles in this shape is " + getSumOfAngles() + " degrees.\n"; 
            
            return desc;  
		}
    }
}