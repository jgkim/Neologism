package com.example.programmingas3.geometricshapes 
{
	public class GeometricShapeFactory 
	{
	   	public static var currentShape:IGeometricShape;

	    public static function createShape(shapeName:String, len:Number):IGeometricShape
	    {
	        switch (shapeName)
	        {       
                case "Triangle":
                    return new EquilateralTriangle(len);
                    
                case "Square":
                    return new Square(len);
                    
	            case "Circle":
	                return new Circle(len);
            }
            return null;
	    }
	    
        public static function describeShape(shapeType:String, shapeSize:Number):String
        {
            GeometricShapeFactory.currentShape = GeometricShapeFactory.createShape(shapeType, shapeSize);
            return GeometricShapeFactory.currentShape.describe();
        }
	}
}