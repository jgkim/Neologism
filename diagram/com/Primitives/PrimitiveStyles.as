//background
[Style(name="backgroundAlpha", type="Number", inherit="no")]
[Style(name="backgroundColor", type="uint", format="Color", inherit="no")]
[Style(name="backgroundSelectedColor", type="uint", format="Color", inherit="no")]

//border
[Style(name="borderColor", type="uint", format="Color", inherit="no")]
[Style(name="borderThickness", type="Number", format="Length", inherit="no")]
[Style(name="borderAlpha", type="Number", inherit="no")]
[Style(name="borderPixelHinting", type="Boolean", inherit="no")]
[Style(name="borderJointStyle", type="String", enumeration="bevel,miter,round", inherit="no")]
[Style(name="borderSelectedColor", type="uint", format="Color", inherit="no")]

//shadow
[Style(name="shadowEnabled", type="Boolean", inherit="no")]
[Style(name="shadowColor", type="uint", format="Color", inherit="yes")]
[Style(name="shadowAngle", type="String", enumeration="left,center,right", inherit="no")]
[Style(name="shadowDistance", type="Number", format="Length", inherit="no")]
[Style(name="shadowAlpha", type="Number", inherit="no")]

//border gradient
[Style(name="borderFillEnabled", type="Boolean",enumeration="true,false", inherit="no")]
[Style(name="borderRatios", type="Array", arrayType="Number", inherit="no")]
[Style(name="borderFillType", type="string",enumeration="linear,radial", inherit="no")]
[Style(name="borderSpreadMethod", type="string",enumeration="pad,reflect,repeat", inherit="no")]
[Style(name="borderFocalPointRatio", type="Number", inherit="no")]
[Style(name="borderAlphas", type="Array",arrayType="Number", inherit="no")]
[Style(name="borderColors", type="Array", arrayType="uint", format="Color", inherit="no")]
[Style(name="borderSelectedColors", type="Array", arrayType="uint", format="Color", inherit="no")]
[Style(name="borderAngle", type="Number", inherit="no")]

//fill gradient
[Style(name="fillEnabled", type="Boolean",enumeration="true,false", inherit="no")]
[Style(name="fillRatios", type="Array", arrayType="Number", inherit="no")]
[Style(name="fillType", type="string",enumeration="linear,radial", inherit="no")]
[Style(name="fillSpreadMethod", type="string",enumeration="pad,reflect,repeat", inherit="no")]
[Style(name="fillFocalPointRatio", type="Number", inherit="no")]
[Style(name="fillAlphas", type="Array", arrayType="Number", inherit="no")]
[Style(name="fillColors", type="Array", arrayType="uint", format="Color", inherit="no")]
[Style(name="fillSelectedColors", type="Array", arrayType="uint", format="Color", inherit="no")]
[Style(name="fillAngle", type="Number", inherit="no")]