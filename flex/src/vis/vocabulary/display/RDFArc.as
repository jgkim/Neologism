package vis.vocabulary.display
{
	
	public class RDFArc
	{
		public static var RDFS_SUBBCLASSOF:String = 'rdfs:subClassOf';
		public static var RDFS_PROPERTY:String = 'rdfs';
		
		private var _label:RDFPropertyLabel;
		private var _domain:RDFNode;
		private var _range:RDFNode;
		
		private var _index:Number;
		private var _type:String;
		
		public function RDFArc(txt:String,d:RDFNode,r:RDFNode,type:String)
		{
			if (txt==null) {
				this._label = null;
			} else {
				this._label = new RDFPropertyLabel();
				this._label.text = txt;
			}
			this._domain = d;
			this._range = r;
			this._index = 0;
			this._type = type;
		}
		
		public function get text() : String {
			if (_label != null) 
				return _label.text;
			return null;
		}
		
		public function get type() : String {
			return _type;
		}
		
		public function set text(t:String) : void {
			if (_label != null)
				_label.text = t;
		}
		
		public function get domain() : RDFNode {
			return _domain;
		}
		
		public function get range() : RDFNode {
			return _range;
		}
		
		public function get label() : RDFPropertyLabel {
			return _label;
		}
		
		public function get index() : Number {
			return _index;
		}
		
		public function set index(idx:Number) : void {
			_index = idx;
		}
	}
}