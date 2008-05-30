package vis.vocabulary.display
{
	import flash.filters.BitmapFilterQuality;
	import flash.filters.BitmapFilterType;
	import flash.filters.DropShadowFilter;
	import flash.filters.GradientGlowFilter;
	
	import mx.containers.Canvas;
	import mx.containers.HBox;
	import mx.containers.VBox;
	import mx.controls.Label;
	import mx.core.ScrollPolicy;
	
	import vis.vocabulary.RDFProperty;

	public class RDFNode extends Canvas
	{
		[ArrayElementType("vis.vocabulary.RDFProperty")]
		private var _rdfName:String;
		private var _rdfLabel:String;
		private var _uiLabel:Label;
		private var _ext:Boolean;
		
		public function RDFNode(_name:String,_label:String,_ext:Boolean)
		{
			super();
			
			this._ext = _ext;
			this._rdfName = _name;
			this._rdfLabel = _label;
			
			var brdClr:Number = 0xC4A000;
			var backClr:Number = 0xFCE94F;
			if (this._ext) {
				brdClr = 0x555753;
				backClr = 0xd3d7cf;
			}
			
			this.setStyle('cornerRadius',5);
			this.setStyle('borderStyle','solid');
			this.setStyle('borderThickness',2);
			this.setStyle('borderColor',brdClr);
			this.setStyle('backgroundColor',backClr);
			this.setStyle('backgroundAlpha',1);
			
			//this.alpha = .7;
			
			this.horizontalScrollPolicy = ScrollPolicy.OFF;
			this.verticalScrollPolicy = ScrollPolicy.OFF;
			
			this.percentHeight = 100;
			this.percentWidth = 100;
			
			this.applyFilters();	
			
			this.setupUi();
		}
		
		public function get rdfName() : String
		{
			return this._rdfName;
		}
		
		public function get rdfLabel() : String
		{
			return this._rdfLabel;
		}
		
		public function set rdfLabel(_label:String) : void
		{
			this._rdfLabel = _label;
		}
		
		public function get extenal() : Boolean
		{
			return this._ext;
		}
		
		private function get gradientGlowFilter() : GradientGlowFilter
		{
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
		
		private function applyFilters() : void
		{
			var _filters:Array = new Array();
			//_filters.push(this.shadowFilter(true));
			_filters.push(this.shadowFilter(false));
			//_filters.push(this.gradientGlowFilter);
			this.filters = _filters;	
		}
		
		private function setupUi() : void
		{
			// custom ui code setup
			_uiLabel = new Label();
			_uiLabel.text = this.rdfLabel;
			_uiLabel.setStyle('textAlign','center');
			_uiLabel.setStyle('fontSize',14);
			_uiLabel.setStyle('fontWeight','bold');
			_uiLabel.setStyle('color',0x204a87);
			_uiLabel.percentWidth = 100;
			
			var hbox:HBox = new HBox();
			hbox.percentWidth = 100;
			hbox.addChild(_uiLabel);
			
			var vbox:VBox = new VBox();
			vbox.percentHeight = 100;
			vbox.percentWidth = 100;
			
			vbox.addChild(hbox);
			this.addChild(vbox);
		}
		
		override protected function updateDisplayList(unscaledWidth:Number, unscaledHeight:Number):void
		{
			super.updateDisplayList(unscaledWidth,unscaledHeight);
			// do my custom paint
		}
		
		public function get handle() : RDFObjectHandles {
			if (this.parent is RDFObjectHandles) {
				return RDFObjectHandles(this.parent);	
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
	}
}