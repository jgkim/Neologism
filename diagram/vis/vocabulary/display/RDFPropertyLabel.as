package vis.vocabulary.display
{
	import mx.controls.Label;
	import flash.events.MouseEvent;
	import flash.filters.DropShadowFilter;

	public class RDFPropertyLabel extends Label
	{
		public static var PALETTE_TANGO:Array = [
			0xfce94f, 0xedd400, 0xc4a000,
			0xfcaf3e, 0xf57900, 0xce5c00,
			0xe9b96e, 0xc17d11, 0x8f5902,
			0x8ae234, 0x73d216, 0x4e9a06,
			0x729fcf, 0x3465a4, 0x204a87,
			0xad7fa8, 0x75507b, 0x5c3566,
			0xef2929, 0xcc0000, 0xa40000,
			0xeeeeec, 0xd3d7cf, 0xbabdb6,
			0x888a85, 0x555753, 0x2e3436
		];
		
		private var _drawBackground:Boolean;
		
		
		public function RDFPropertyLabel()
		{
			super();
			this._drawBackground = true;
			//this.setStyle('color',0x204a87);
			//this.setStyle('fontSize',10); 
			//this.setStyle('fontWeight','bold');
			//this.setStyle('textAlign','center');
			//this.addEventListener(MouseEvent.CLICK,this.onClick);
			this.applyFilters();
		}
		
		private function shadowFilter(inner:Boolean,knok:Boolean = false) : DropShadowFilter
		{
			var dropShadow:DropShadowFilter = new DropShadowFilter();
			dropShadow.distance = 7;
			dropShadow.angle = 45;
			dropShadow.alpha = .2;
			dropShadow.inner = inner;
			dropShadow.knockout = knok;
			
			return dropShadow;
		}
		
		protected function applyFilters() : void
		{
			var _filters:Array = new Array();
			_filters.push(shadowFilter(false));
			this.filters = _filters;
		}
		
		public function get drawBackground() : Boolean
		{
			return this._drawBackground;
		}
		
		public function set drawBackground(show:Boolean) : void
		{
			this._drawBackground = show;
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number): void
    	{
    		if (_drawBackground) {
	    		var clr:uint = 0xeeeeec;
	    		graphics.beginFill(clr,.7);
	    		graphics.drawRoundRect(0,0,this.width,this.height,3);
	    		graphics.endFill();
    		}
    		super.updateDisplayList(unscaledWidth, unscaledHeight);
    	}
    	
    	//private function onClick(event:MouseEvent) : void
    	//{
    	//}
		
	}
}