package vis.vocabulary.display
{
	import com.roguedevelopment.objecthandles.ObjectHandleEvent;
	import com.roguedevelopment.objecthandles.ObjectHandles;
	
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	import mx.containers.Canvas;
	import mx.core.UIComponent;

	public class RDFObjectHandles extends ObjectHandles
	{
		private var _enableSelection:Boolean = true;
		private var _padd:Number;
		
		public function RDFObjectHandles()
		{
			super();
			_padd = 7;
			selectionModeUpdate();
		}
		
		public function get enableSelection() : Boolean
		{
			return this._enableSelection;
		}
		
		public function set enableSelection(enable:Boolean) : void
		{
			this._enableSelection = enable;
			this.selectionModeUpdate();
		}
		
		protected function selectionModeUpdate() : void
		{
			if (this._enableSelection) {
				this.addEventListener( MouseEvent.MOUSE_DOWN, this.onMouseDown );
				this.addEventListener( MouseEvent.MOUSE_UP, this.onMouseUp );		
				this.addEventListener( MouseEvent.MOUSE_OVER, this.onMouseOver );			
				this.addEventListener( MouseEvent.MOUSE_OUT, this.onMouseOut );
				this.addEventListener( MouseEvent.MOUSE_MOVE, this.onMouseMove );
				this.addEventListener(ObjectHandleEvent.OBJECT_RESIZING_EVENT, this.onResizing);
				//this.addEventListener(ObjectHandleEvent.OBJECT_RESIZED_EVENT, this.onResize);
				this.mouseChildren = false;
			} else {
				this.removeEventListener( MouseEvent.MOUSE_DOWN, this.onMouseDown );
				this.removeEventListener( MouseEvent.MOUSE_UP, this.onMouseUp );		
				this.removeEventListener( MouseEvent.MOUSE_OVER, this.onMouseOver );			
				this.removeEventListener( MouseEvent.MOUSE_OUT, this.onMouseOut );
				this.removeEventListener( MouseEvent.MOUSE_MOVE, this.onMouseMove );
				this.removeEventListener(ObjectHandleEvent.OBJECT_RESIZING_EVENT, this.onResizing);
				//this.removeEventListener(ObjectHandleEvent.OBJECT_RESIZED_EVENT, this.onResize);
				this.mouseChildren = true;
			}
		}
		
		public function get rdfNode(): RDFNode
		{
			if( this.getChildAt(0) is RDFNode ) {
				return RDFNode( this.getChildAt(0) );
			}
			return null;
		}
		
		public function get cx() : Number
		{
			return int(x+width/2);
		}
		
		public function get cy() : Number
		{
			return int(y+height/2);
		}
		
		public function get padd() : Number
		{
			return _padd;
		}
		
		public function get paddRectangle() : Rectangle
		{
			return new Rectangle(x-_padd,y-_padd,width+2*_padd,height+2*_padd);
		}
		
		public function arcInPoint(handler:RDFObjectHandles): Point
		{
			return Utils.intersectRect(paddRectangle, handler.cx, handler.cy);
		}
		
		[ArrayElementType("Point")]
		public function arcOnItself() : Array {
			return Utils.arcOnRect(paddRectangle);
		}
		
		// added by guidocecilio - 09 July 2009
		// added a mouse listener to prevent the nodes overflow
		// this fix the problem when the user move a class node outside the left and the top margin
		override protected function onMouseMove(event:MouseEvent) : void
		{
			trace(event.target);
			
			if( !visible ) { return; }
			
			if( !event.buttonDown )
			{
				setMouseCursor( event.stageX, event.stageY );
				return;
			}
			
			if( this.parent is Canvas ) {
				var parentCanvas:Canvas = parent as Canvas;
				trace(parentCanvas.name + ", x = "  + parentCanvas.x + ", width = " + parentCanvas.width);
				trace(this.name + ", x = "  + this.x + ", width = " + this.width + ", y = " + this.y + ", height = " + this.height);
				
				super.onMouseMove(event);
				
				if( this.x < 0 ) this.x = 0;
				if( this.y < 0 ) this.y = 0;
			}
			
		}

		protected function onResizing(event:Event) : void
		{
			//var child:UIComponent = this.getChildAt(0) as UIComponent;
			//child.invalidateSize();
			invalidateSize();
		}
		
		public function getX():int {
			return this.x;
		}
		
		public function getY():int {
			return this.y;	
		}
		
		public function getWidth():int {
			return this.width;	
		}
		
		public function getHeight():int {
			return this.height;	
		}
	}
}