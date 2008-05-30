package vis.vocabulary.display
{
	import com.roguedevelopment.objecthandles.ObjectHandles;
	
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.geom.Rectangle;

	public class RDFObjectHandles extends ObjectHandles
	{
		private var _enableSelection:Boolean = true;
		private var _padd:Number;
		
		public function RDFObjectHandles()
		{
			super();
			_padd = 7;
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
				this.mouseChildren = false;
			} else {
				this.removeEventListener( MouseEvent.MOUSE_DOWN, this.onMouseDown );
				this.removeEventListener( MouseEvent.MOUSE_UP, this.onMouseUp );		
				this.removeEventListener( MouseEvent.MOUSE_OVER, this.onMouseOver );			
				this.removeEventListener( MouseEvent.MOUSE_OUT, this.onMouseOut );
				this.removeEventListener( MouseEvent.MOUSE_MOVE, this.onMouseMove );
				this.mouseChildren = true;
			}
		}
		
		public function get rdfNode() : RDFNode
		{
			if (this.getChildAt(0) is RDFNode) {
				return RDFNode(this.getChildAt(0));
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
		
		public function arcInPoint(handler:RDFObjectHandles) : Point
		{
			return Utils.intersectRect(paddRectangle,handler.cx,handler.cy);
		}
		
		[ArrayElementType("Point")]
		public function arcOnItself() : Array {
			return Utils.arcOnRect(paddRectangle);
		}
	}
}