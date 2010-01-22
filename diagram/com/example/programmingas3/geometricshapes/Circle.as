package com.example.programmingas3.geometricshapes 
{
	public class Circle implements IGeometricShape 
	{
		public var diameter:Number;
		
		public function Circle(diam:Number = 100):void
		{
			this.diameter = diam;
		}
		
		public function getArea():Number
		{
		    // the formula is Pi * radius^2
		    return Math.PI * ((diameter / 2)^2);
		}
		
		public function getCircumference():Number
		{
		    // the formula is Pi * radius * 2
		    return Math.PI * diameter;
		}
		
		public function describe():String
		{
            var desc:String = "This shape is a Circle.\n";
            desc += "Its diameter is " + diameter + " pixels.\n";
            desc += "Its area is " + getArea() + ".\n";
            desc += "Its circumference is " + getCircumference() + ".\n";    
            return desc;
		}
	}
}