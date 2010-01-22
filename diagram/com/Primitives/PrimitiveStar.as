package com.Primitives
{
	import mx.core.UIComponent;
	import com.Primitives.DrawUtils; 
	import com.Primitives.BasePrimitive;
	
	public class PrimitiveStar extends BasePrimitive
	{
		public function PrimitiveStar():void
		{
			super();
		}		
		
		[Bindable]
		[Inspectable]
		private var _outerRadius:Number=50;
		public function set outerRadius(value:Number):void
		{
			_outerRadius = value;
			
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get outerRadius():Number
		{
			return _outerRadius;
		}
		
		[Bindable]
		[Inspectable]
		private var _innerRadius:Number=25;
		public function set innerRadius(value:Number):void
		{
			_innerRadius = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get innerRadius():Number
		{
			return _innerRadius;
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
									
			//get the ratio of the difference for the inner and outer radius	
			var ratio:Number= outerRadius/innerRadius; 
			
			//use the smallest of the width and height to do the 
			//size so we can use as much space as possible
			var maxSize:Number = Math.min(unscaledHeight, unscaledWidth);
			DrawUtils.star(this,unscaledWidth/2,unscaledHeight/2,points,(maxSize/2)/ratio,(maxSize/2),angle);
				
		}
		
	}
		
}