package com.Primitives
{
	import mx.core.UIComponent;
	import com.Primitives.DrawUtils; 
	import com.Primitives.BasePrimitive;
	import flash.events.MouseEvent;
	import flash.events.Event;
	import mx.controls.Label;
	
	
	public class PrimitivePolygon extends BasePrimitive
	{
		public function PrimitivePolygon():void
		{
			super();
		}		
		
		
		[Bindable]
		[Inspectable]
		private var _radius:Number=50;
		public function set radius(value:Number):void
		{
			_radius = value;
			
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get radius():Number
		{
			return _radius;
		}
		
		[Bindable]
		[Inspectable]
		private var _points:Number=5;
		public function set points(value:Number):void
		{
			_points = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get points():Number
		{
			return _points;
		}
		
		[Bindable]
		[Inspectable]
		private var _angle:Number = 90;
		public function set angle(value:Number):void
		{
			_angle = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get angle():Number
		{
			return _angle;
		}
		
		override protected function updateDisplayList(unscaledWidth:Number,unscaledHeight:Number):void
		{												
			super.updateDisplayList(unscaledWidth,unscaledHeight);
			
			//use the smallest of the width and height to do the 
			//size so we can use as much space as possible
			radius = Math.min(unscaledHeight/2, unscaledWidth/2);
			DrawUtils.polygon(this,unscaledWidth/2,unscaledHeight/2,points,radius,angle);
		}
	}
		
}