package vis.vocabulary.display
{
	/*
	look at :
	
		http://www.faqs.org/faqs/graphics/algorithms-faq/
		
	*/
	
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	public final class Utils
	{
		// returns angle in radians
		public static function lineAngle(p1:Point, p2:Point) : Number
        {
            var ang:Number = 0;
            var dx:Number = p1.x - p2.x;
            var dy:Number = p1.y - p2.y;

            var rad:Number = Math.atan(dy / dx);
            ang = 180 * rad / Math.PI;

            if (dy <= 0)
                ang = (ang < 0) ? 180 + ang : ang;
            else
                ang = (ang > 0) ? 180 + ang : 360 + ang;

            return ang * Math.PI / 180;
        }
			
		[ArrayElementType("Point")]	
		public static function arrowHeads(p2:Point, mp2:Point, ylen:Number = 4, xlen:Number = 7): Array {
			var mat:Matrix = new Matrix();
			var alfa:Number = Utils.lineAngle(p2, mp2);
			var a1:Point = new Point(xlen,-ylen);
			var a2:Point = new Point(xlen,+ylen);
			
			mat.rotate(alfa);
			mat.translate(p2.x,p2.y);
			
			a1 = mat.transformPoint(a1);
			a2 = mat.transformPoint(a2);
			
			return [a1,a2];
		}
		
		[ArrayElementType("Point")]	
		public static function arrowHeads2(A:Point,B:Point,hlen:Number=5,plen:Number=0.08) : Array {
			var L:Number = Math.sqrt((B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(B.y-A.y));
			var D1:Number = hlen/L;
			var D2:Number = -D1;
			var r:Number = plen;
			
			var x1:Number = -1;
			var y1:Number = -1;
			var x2:Number = -1;
			var y2:Number = -1;
			
			x1 = (B.x-A.x)*r + (B.y-A.y)*D1 + A.x;
			x2 = (B.x-A.x)*r + (B.y-A.y)*D2 + A.x;
				
			if (A.y == B.y) {
				y1 = B.y + hlen;
				y2 = B.y - hlen;
			} else {
				y1 = (r*L*L - r*(B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(A.y-D1*(B.x-A.x))) / (B.y-A.y);
				y2 = (r*L*L - r*(B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(A.y-D2*(B.x-A.x))) / (B.y-A.y);
			}
			return [new Point(x1,y1),new Point(x2,y2)];
		}
		
		public static function middlePerpendicular2(p1:Point, p2:Point,d:Number,cw:Boolean) : Point {
			var m:Point = Utils.middle(p1,p2);
			var p:Point = Utils.middlePerpendicular(p1,p2,d,cw);
			return Utils.middle(m,p);
		}
		
		public static function middlePerpendicular(p1:Point, p2:Point,d:Number,cw:Boolean) : Point {
			var p:Point = null;
			var m:Point = Utils.middle(p1,p2);
			var mat:Matrix = new Matrix();
			var alfa:Number = Utils.lineAngle(p1,p2);
			
			mat.translate(-p1.x,-p1.y);
			mat.rotate(-alfa);
			
			p = mat.transformPoint(m);
			p.y += ((cw) ? 1 : -1) * d;
			
			mat.identity();
			mat.rotate(alfa);
			mat.translate(p1.x,p1.y);
			
			return mat.transformPoint(p);
		}
		
		public static function middlePerpendicularPoint2(p1:Point, p2:Point,d:Number,cw:Number) : Point {
			var m:Point = Utils.middle(p1,p2);
			var p:Point = Utils.middlePerpendicularPoint(p1,p2,d,cw);
			return Utils.middle(m,p);
		}
		
		public static function middlePerpendicularPoint(A:Point,B:Point,d:Number,cw:Number) : Point {
			var L:Number = Math.sqrt((B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(B.y-A.y));
			var D:Number = (cw < 0) ? d/L*(-1) : d/L;
			var x:Number = (B.x-A.x)*.5 + (B.y-A.y)*D + A.x;			
			var y:Number = 0;
			if (A.y == B.y) {
				y = B.y + D;
			} else {
			 	y = (.5*L*L - .5*(B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(A.y-D*(B.x-A.x))) / (B.y-A.y);
			}
			return new Point(x,y);
		}
		
		// fast
		[ArrayElementType("Point")]
		public static function arcOnRect(r:Rectangle) : Array {
			//TODO hardcoded on Q2
			return [new Point(r.left+r.width/2,r.top), new Point(r.right,r.top + r.height /2)];
		}
		
		// fast
		public static function middle(p1:Point, p2:Point) : Point 
		{
			return new Point((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
		}
		
		// fast
		public static function intersect(x1:Number,y1:Number,x2:Number,y2:Number,x3:Number,y3:Number,x4:Number,y4:Number) : Point
		{
			// see algorithm @ http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline2d/
			var result:Point = null;
			var d:Number = ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
			
			if (d != 0) {
				var ua:Number = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / d;
				var ub:Number = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / d;
				
				if (ua > 0 && ua < 1 && ub > 0 && ub < 1) {
					var x:Number = x1 + ua*(x2-x1);
					var y:Number = y1 + ua*(y2-y1);
					result = new Point(x,y);
				}	
			}
			return result;
		}
		
		// fast
		public static function intersectRect(rect:Rectangle,x:Number,y:Number) : Point
		{
			var cx:Number = int(rect.x+rect.width/2);
			var cy:Number = int(rect.y+rect.height/2);
			var results:Array = new Array();
			
			results.push( Utils.intersect(rect.left,rect.bottom,rect.left,rect.top,cx,cy,x,y) );
			results.push( Utils.intersect(rect.left,rect.top,rect.right,rect.top,cx,cy,x,y) );
			results.push( Utils.intersect(rect.right,rect.top,rect.right,rect.bottom,cx,cy,x,y) );
			results.push( Utils.intersect(rect.right,rect.bottom,rect.left,rect.bottom,cx,cy,x,y) );
			
			for each (var pt:Point in results) {
				if ( pt != null) {
					return pt;
				}
			}
			return null;
		}
		
		// fast --- shows position of point according to line -1 left of AB, 1 right of AB, 0 on AB
		public static function ccwPoint2Line(C:Point, A:Point, B:Point) : Number
		{
			var L:Number = Math.sqrt((B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(B.y-A.y));
			var s:Number = ((A.y-C.y)*(B.x-A.x) - (A.x-C.x)*(B.y-A.y)) / L*L;
			
			return s / Math.abs(s);
		}
		
		// fast
		public static function distPoint2Line(C:Point, A:Point, B:Point) : Number
		{
			var L:Number = Math.sqrt((B.x-A.x)*(B.x-A.x) + (B.y-A.y)*(B.y-A.y));
			var r:Number = ((C.x-A.x)*(B.x-A.x) + (C.y-A.y)*(B.y-A.y)) / L*L;
			var s:Number = ((A.y-C.y)*(B.x-A.x) - (A.x-C.x)*(B.y-A.y)) / L*L;
			
			return Math.abs(s) * L;
		}
		
		public static function distance(a:Point, b:Point):Number {
			return Math.sqrt( Math.pow(b.x - a.x, 2) + Math.pow(b.y - a.y, 2) );
		}
	}
}