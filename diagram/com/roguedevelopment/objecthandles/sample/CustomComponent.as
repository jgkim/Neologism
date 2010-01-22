package com.roguedevelopment.objecthandles.sample
{
	import com.roguedevelopment.objecthandles.ObjectHandleEvent;
	import com.roguedevelopment.objecthandles.ObjectHandles;
	
	import flash.events.Event;
	
	import mx.events.ResizeEvent;

	public class CustomComponent extends ObjectHandles
	{
		public function CustomComponent()
		{
			super();
			
			addEventListener(ObjectHandleEvent.OBJECT_RESIZING_EVENT, onResizing );
			addEventListener(ResizeEvent.RESIZE, onResizing );
			redraw();
		}
		
		protected function onResizing(e:Event) : void
		{
			redraw(); 
		}
		
		protected function redraw() : void
		{
			graphics.clear();
			graphics.lineStyle(1,0);
			graphics.moveTo(0,0);
			graphics.lineTo(width,height);
			graphics.moveTo(width,0);
			graphics.lineTo(0,height);
			
		}
		
	}
}