package vis.vocabulary.display
{
	import flash.events.Event;
	import flash.events.MouseEvent;
	
	public class RDFPropertyLabelObjectHandles extends RDFObjectHandles
	{
		protected var _pulling:Boolean = false;
		
		public function RDFPropertyLabelObjectHandles()
		{
			super();
		}
		
		override protected function onMouseMove(event:MouseEvent) : void
		{
			if( !visible ) { return; }
			trace('yes moving');
			if( event.buttonDown )	{
				//_pulling = true;
			}
						
			super.onMouseMove(event);
		}
		
		override protected function onMouseDown(event:MouseEvent) : void
		{
			if( event.ctrlKey ) {
				this.dispatchEvent( new Event(RDFArc.LABEL_MANUALLYMOVED, true));
								
				_pulling = true;
			}
			if( !visible ) { return; }
			super.onMouseDown(event);
		}
		
		override protected function onMouseUp(event:MouseEvent) : void
		{
			_pulling = false;
			if( !visible ) { return; }
			super.onMouseUp(event);
		}
		
		override protected function onResizing(event:Event) : void
		{
			_pulling = false;
			super.onResizing(event);
		}
		
		public function get pulling():Boolean {
			return _pulling;
		}
	}
}