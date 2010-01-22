package vis.vocabulary
{
	import mx.controls.Label;
	
	import vis.vocabulary.display.RDFNode;
	import vis.vocabulary.display.RDFPropertyLabel;
	
	public class RDFProperty
	{
		private var _rdfName:String;
		private var _rdfLabel:String;
		
		[ArrayElementType("vis.vocabulary.display.RDFNode")]
		private var _domain:Array;
		[ArrayElementType("vis.vocabulary.display.RDFNode")]
		private var _range:Array;
		
		public function RDFProperty(_name:String,_label:String) {
			this._rdfName = _name;
			this._rdfLabel = _label;
			
			this._domain = new Array();
			this._range = new Array();	
		}
		
		public function get singleDomain() : RDFNode
		{
			if (_domain[0] is RDFNode)
				return RDFNode(_domain[0]);
			return null;
		}
		
		public function get singleRange() : RDFNode
		{
			if (_range[0] is RDFNode)
				return RDFNode(_range[0]);
			return null;
		}
		
		public function get domain() : Array
		{
			return this._domain;
		}
		
		public function get range() : Array
		{
			return this._range;
		}
		
		public function set domain(d:Array) : void
		{
			this._domain = new Array();
			for each (var e:String in d) {
				_domain.push(e);
			}
		}
		
		public function set range(r:Array) : void
		{
			this._range = new Array();
			for each (var e:String in r) {
				_range.push(e);
			}
		}
		
		public function get rdfName() : String
		{
			return this._rdfName;
		}
		
		public function set rdfName(_name:String) : void
		{
			this._rdfName = _name;
		}
		
		public function get rdfLabel() : String
		{
			return this._rdfLabel;
		}
		
		public function set rdfLabel(_label:String) : void
		{
			this._rdfLabel = _label;
		}
		
		public function get hasDomain() : Boolean{
			if (_domain.length > 0)
				return true;
			return false;	
		}
		
		public function get hasRange() : Boolean{
			if (_range.length > 0)
				return true;
			return false;	
		}
	}
}