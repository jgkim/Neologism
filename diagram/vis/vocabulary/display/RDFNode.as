package vis.vocabulary.display
{
	import flash.filters.BitmapFilterQuality;
	import flash.filters.BitmapFilterType;
	import flash.filters.DropShadowFilter;
	import flash.filters.GradientGlowFilter;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFormat;
	import flash.text.TextFormatAlign;
	
	import mx.core.UIComponent;

	public class RDFNode extends UIComponent
	{
		//[ArrayElementType("vis.vocabulary.RDFProperty")]
		protected var _rdfName:String;
		protected var _rdfLabel:String;
		protected var _external:Boolean;
		
		private var _label:TextField;
		private var _borderColor:int = 0xC4A000;
		private var _backgroundColor:int = 0xFCE94F;
		private var _renderLabel:Boolean = true;
		private var _renderNodeShadow:Boolean = true;
		
		public function RDFNode(_name:String,_label:String,_ext:Boolean)
		{
			super();
			
			_external = _ext;
			_rdfName = _name;
			_rdfLabel = _label;
			
			if( _external ) {
				_borderColor = 0x555753;
				_backgroundColor = 0xd3d7cf;
			}
		}
		
		override protected function createChildren():void {
			this.percentHeight = 100;
			this.percentWidth = 100;
			
			configureLabel();
            setLabel(_rdfLabel);
			this.applyFilters();
		}
		
		// Implement the commitProperties() method. 
		override protected function commitProperties():void {
		    super.commitProperties();
		    
		}
		
		override protected function measure():void {
            super.measure();
            trace("RDFNode::measure called");
        }
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void
		{
			trace("RDFNode::updateDisplayList called")
			
			this.graphics.clear();
			
			this.graphics.lineStyle(2, _borderColor, 1.0);
    		this.graphics.drawRoundRect(0, 0, unscaledWidth, unscaledHeight, 15); 
	    	this.graphics.beginFill(_backgroundColor,.7);
	    	this.graphics.drawRoundRect(0, 0, unscaledWidth, unscaledHeight, 15);
	    	this.graphics.endFill();
	    	
	    	_label.width = unscaledWidth;
            _label.height = unscaledHeight;
            //_label.x = (Math.abs(x + width - x) / 2) - _label.textWidth / 2 - 2;
			_label.y = (Math.abs(y + height - y) / 2) - _label.textHeight / 2 - 2;
			// this avoid the negatives values when _label.textWidth or _label.textHeight 
			// are greater than width and height respectively 
			if( _label.x < 0 ) _label.x = 0;
			if( _label.y < 0 ) _label.y = 0;
			
			trace("parent.width = " + parent.width + ", y = " + y);
			trace("unscaledWidth = " + unscaledWidth);
			trace("label.y = " + _label.y + "_label.textHeight = " + _label.textHeight);

	   		super.updateDisplayList(unscaledWidth, unscaledHeight);
		}
		
		public function setLabel(str:String):void {
            _label.text = str;
        }
		
		private function configureLabel():void {
            _label = new TextField();
            _label.width = 0;
            _label.height = 0;
			_label.x = 0;
			_label.y = 0;
            
            _label.background = false;
            //_label.border = true;
            //_label.multiline = true;
            _label.wordWrap = true;
            _label.selectable = false;
            //_label.autoSize = TextFieldAutoSize.CENTER;
            _label.autoSize = TextFieldAutoSize.NONE;
            //_label.textColor = 0xff0000;
            
            var format:TextFormat = new TextFormat();
            format.font = "Verdana";
            format.color = 0x645300;//0x555753;
            format.size = 12;
            format.align = TextFormatAlign.CENTER;
                        
            _label.setTextFormat(format);
            _label.defaultTextFormat = format;
            addChild(_label);
        }

		public function get rdfName(): String {
			return this._rdfName;
		}
		
		public function get rdfLabel(): String {
			return this._rdfLabel;
		}
		
		public function set rdfLabel(_label:String): void {
			this._rdfLabel = _label;
		}
		
		public function get extenal(): Boolean {
			return this._external;
		}
		
		private function get gradientGlowFilter(): GradientGlowFilter {
			var gradientGlow:GradientGlowFilter = new GradientGlowFilter();
			gradientGlow.distance = 0;
			gradientGlow.angle = 45;
			gradientGlow.colors = [0x000000, 0x204a87];
			gradientGlow.alphas = [0, 1];
			gradientGlow.ratios = [0, 255];
			gradientGlow.blurX = 50;
			gradientGlow.blurY = 50;
			gradientGlow.strength = 0.5;
			gradientGlow.quality = BitmapFilterQuality.HIGH;
			gradientGlow.type = BitmapFilterType.OUTER;
			gradientGlow.knockout = false;
			
			return gradientGlow;
		} 
		
		private function shadowFilter(inner:Boolean,knok:Boolean = false): DropShadowFilter {
			var dropShadow:DropShadowFilter = new DropShadowFilter();
			dropShadow.distance = 7;
			dropShadow.angle = 45;
			dropShadow.alpha = .2;
			dropShadow.inner = inner;
			dropShadow.knockout = knok;
			
			return dropShadow;
		}
		
		private function applyFilters(): void {
			var _filters:Array = new Array();
			//_filters.push(this.shadowFilter(true));
			_filters.push(this.shadowFilter(false));
			//_filters.push(this.gradientGlowFilter);
			this.filters = _filters;	
		}
				
		public function get objectHandle(): RDFObjectHandles {
			if (this.parent is RDFObjectHandles) {
				return RDFObjectHandles(this.parent);	
			}
			return null;
		}
		
		public function get cx(): Number {
			return int(x+width/2);
		}
		
		public function get cy(): Number {
			return int(y+height/2);
		}
		
		public function set renderShadow(value:Boolean):void {
			_renderNodeShadow = value;
			if( !_renderNodeShadow) {
				this.filters = null;
			} else {
				applyFilters();
			}
		}
		
		public function get renderShadow():Boolean {
			return _renderNodeShadow;	
		}
	}
}