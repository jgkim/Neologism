package com.Primitives
{
	import mx.core.UIComponent;
	import com.Primitives.DrawUtils; 
	import com.Primitives.BasePrimitive;
	import flash.geom.Point;
					
	public class PrimitiveArc extends BasePrimitive
	{
		public function PrimitiveArc():void
		{
			super();
			super.backgroundEnabled=false;
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
		private var _yRadius:Number=0;
		public function set yRadius(value:Number):void
		{
			_yRadius = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get yRadius():Number
		{
			return _yRadius;
		}
		
		[Bindable]
		[Inspectable]
		private var _startAngle:Number = 90;
		public function set startAngle(value:Number):void
		{
			_startAngle = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get startAngle():Number
		{
			return _startAngle;
		}
		
		[Bindable]
		[Inspectable]
		private var _arc:Number = 90;
		public function set arc(value:Number):void
		{
			_arc = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get arc():Number
		{
			return _arc;
		}
									
		override protected function updateDisplayList(unscaledWidth:Number,unscaledHeight:Number):void
		{
												
			super.updateDisplayList(unscaledWidth,unscaledHeight);
							
			//use the smallest of the width and height to get the radius.
			radius = Math.min(unscaledHeight, unscaledWidth)/2;
			
			if (yRadius==0)
			{
				yRadius = radius;
			}
						
			
			//calculate based on startangle, radius, x, and y position to start the drawing
			var newX:Number = (unscaledWidth/2) + (unscaledWidth/2) * 
			Math.cos(startAngle * (Math.PI / 180));
			
			var newY:Number = (unscaledWidth/2) - (unscaledWidth/2) * 
			Math.sin(startAngle * (Math.PI / 180));
			
			
			DrawUtils.arcTo(this,newX,newY,startAngle,arc,radius,yRadius);
						
				
		}
		
	}
		
}