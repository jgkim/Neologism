/*
# ***** BEGIN LICENSE BLOCK *****
Copyright the original author or authors.
Licensed under the MOZILLA PUBLIC LICENSE, Version 1.1 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
	http://www.mozilla.org/MPL/MPL-1.1.html
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# ***** END LICENSE BLOCK *****
*/

package sandy.util
{
	import flash.geom.Matrix;
	
	public class Triangle
	{
		public var p0 : SandyPoint;
		public var p1 : SandyPoint;
		public var p2 : SandyPoint;
		public var tMat : Matrix;
		
		public function Triangle( p0 : SandyPoint, p1 : SandyPoint, p2 : SandyPoint, tMat : Matrix )
		{
			this.p0 = p0;
			this.p1 = p1
			this.p2 = p2;
			this.tMat = tMat;			
		}
	}
}