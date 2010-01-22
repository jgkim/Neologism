package vis.vocabulary.display
{
	import flash.filters.DropShadowFilter;
	import flash.text.TextField;
	import flash.text.TextFormat;
	import flash.text.TextFormatAlign;
	
	import mx.core.UIComponent;

	public class RDFPropertyLabel2 extends UIComponent
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
		
		private var _label:TextField;
		private var _text:String;
		
		private var _renderLabel:Boolean = true;
		private var _renderShadow:Boolean = false;
		private var _renderBackground:Boolean = true;
		
		protected var _textAlign:String = TextFormatAlign.LEFT;
		
		// background box 
		private var _backgroundAlpha:Number = .7;
		//private var _borderColor:int = 0xaaaaaa;
		private var _backgroundColor:int = 0xf6f6f6;//0xaabbff;//0xeeeeec;
		
		//
		private var arcReference:RDFArc = null;
	
		
		public function RDFPropertyLabel2(text:String)
		{
			super();
			_text = text;
			
			width = 50;
			height = 50;
		}
		
		override protected function createChildren():void {
			this.percentHeight = 100;
			this.percentWidth = 100;
			
			configureLabel();
			this._label.text = _text;
			if( _renderShadow ) 
				this.applyFilters();
			
			super.createChildren();
		}
		
		// Implement the commitProperties() method. 
		override protected function commitProperties():void {
		    super.commitProperties();
		    
		}
		
		override protected function measure():void {
            super.measure();
    
            //measuredWidth=100;
            //measuredMinWidth=50;
            //measuredHeight=50;
            //measuredMinHeight=25;
            
           
            trace("RDFNode::measure called");
        }
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number): void
    	{
    		this.graphics.clear();
    		
			if( _renderBackground ) {
				this.graphics.beginFill(_backgroundColor, _backgroundAlpha);
				this.graphics.drawRoundRect(0, 0, unscaledWidth, unscaledHeight, 15);
				this.graphics.endFill();
			}
	    	
	    	_label.width = width;
            _label.height = height;
            _label.x = (Math.abs(x + width - x) / 2) - _label.textWidth / 2;
			_label.y = (Math.abs(y + height - y) / 2) - _label.textHeight / 2;
			// this avoid the negatives values when _label.textWidth or _label.textHeight 
			// are greater than width and height respectively 
			if( _label.x < 0 ) _label.x = 0;
			if( _label.y < 0 ) _label.y = 0;
			
			super.updateDisplayList(unscaledWidth, unscaledHeight);
    	}
		
		public function set text(str:String):void {
            _text = str;
        }
        
        public function get text():String {
            return _text;
        }
		
		public function set textAlign(align:String): void {
			_textAlign = align;
			configureLabel();	
		}
		
		public function get textAlign(): String {
			return _textAlign;	
		}
		
		private function configureLabel():void {
            _label = new TextField();
            _label.width = 0;
            _label.height = 0;
			_label.x = 0;
			_label.y = 0;
            //_label.autoSize = TextFieldAutoSize.CENTER;
            _label.background = false;
            _label.border = false;
            _label.multiline = true;
            _label.wordWrap = true;
            _label.selectable = false;
			            
            var format:TextFormat = new TextFormat();
            format.font = "Verdana";
            format.color = 0x1735AC;//0x555753;
            format.size = 10;
			format.align = _textAlign;
            _label.defaultTextFormat = format;
            addChild(_label);
        }

		private function shadowFilter(inner:Boolean, knok:Boolean = false) : DropShadowFilter
		{
			var dropShadow:DropShadowFilter = new DropShadowFilter();
			dropShadow.distance = 3;
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
		
		public function set renderShadow(value:Boolean):void {
			_renderShadow = value;
			if( !_renderShadow) {
				this.filters = null;
			} else {
				applyFilters();
			}
		}
		
		public function get renderShadow():Boolean {
			return _renderShadow;	
		}
		
		public function set RDFArcReference(arc:RDFArc): void {
			arcReference = arc;
		}
		
		public function get RDFArcReference():RDFArc {
			return arcReference;
		}
		
		public function set renderBackground(value:Boolean):void {
			_renderBackground = value;
		} 
		
		public function get renderBackground():Boolean {
			return _renderBackground;
		}
	}
}