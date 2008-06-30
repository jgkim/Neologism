package com.Primitives
{
	import flash.display.GradientType;
	import flash.display.SpreadMethod;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.filters.DropShadowFilter;
	import flash.geom.*;
	
	import mx.controls.dataGridClasses.DataGridListData;
	import mx.controls.listClasses.BaseListData;
	import mx.controls.listClasses.IDropInListItemRenderer;
	import mx.controls.listClasses.IListItemRenderer;
	import mx.core.EdgeMetrics;
	import mx.core.IDataRenderer;
	import mx.core.IFlexDisplayObject;
	import mx.core.UIComponent;
	import mx.core.mx_internal;
	import mx.events.FlexEvent;
	import mx.managers.IFocusManagerComponent;
	
	
	use namespace mx_internal;	
	
	include "PrimitiveStyles.as";
	
	public class BasePrimitive extends UIComponent
	implements IDataRenderer, IDropInListItemRenderer,
       IFocusManagerComponent, IListItemRenderer
       {		
		public var selectedField:String = null;
		private var selectedSet:Boolean;
		private var styleChangedFlag:Boolean = true;
		 
		public function BasePrimitive()
		{
			mouseChildren = false;
  			
  			//setup the events      	
    	    addEventListener(MouseEvent.ROLL_OVER, rollOverHandler);
	        addEventListener(MouseEvent.ROLL_OUT, rollOutHandler);
        	addEventListener(MouseEvent.MOUSE_DOWN, mouseDownHandler);
    	    addEventListener(MouseEvent.MOUSE_UP, mouseUpHandler);
	        addEventListener(MouseEvent.CLICK, clickHandler);
	        
	        addEventListener(FlexEvent.UPDATE_COMPLETE, handleUpdateComplete);
	        
		}
		
	    private var toolTipSet:Boolean = false;
	    [Inspectable(category="General", defaultValue="null")]
	    override public function set toolTip(value:String):void
	    {
	        super.toolTip = value;
	
	        if (value)
	        {
	            toolTipSet = true;
	        }
	        else
	        {
	            toolTipSet = false;
	            invalidateDisplayList();
	        }
	    }
		
		mx_internal var _selected:Boolean = false;
    	[Bindable("click")]
	    [Bindable("valueCommit")]
    	[Inspectable(category="General", defaultValue="false")]
    
		public function get selected():Boolean
	    {
	        return _selected;
	    }
	    public function set selected(value:Boolean):void
	    {
	        selectedSet = true;
	        setSelected(value);
	    }
	
	    mx_internal function setSelected(value:Boolean):void
	    {
	        if (_selected != value)
	        {
	            _selected = value;
	
	            //invalidateDisplayList();
	    
	            if (toggle)
	                dispatchEvent(new Event(Event.CHANGE));
		            dispatchEvent(new FlexEvent(FlexEvent.VALUE_COMMIT));
	        }
	    }
	    
	    mx_internal var _toggle:Boolean = false;
	    mx_internal var toggleChanged:Boolean = false;
	
	    [Bindable("toggleChanged")]
	    [Inspectable(category="General", defaultValue="false")]
	    public function get toggle():Boolean
	    {
	        return _toggle;
	    }
	    public function set toggle(value:Boolean):void
	    {
	        _toggle = value;
	        toggleChanged = true;
	
	        invalidateProperties();
	        invalidateDisplayList();
	
	        dispatchEvent(new Event("toggleChanged"));
	    }
	
	
	    private var _data:Object;
	
	    [Bindable("dataChange")]
	    [Inspectable(environment="none")]
	    public function get data():Object
	    {
	        if (!_data)
	            _data = {};
	
	        return _data;
	    }
	
	   
	    public function set data(value:Object):void
	    {
	        var newSelected:*;
	    
	        _data = value;
	
	        if (_listData && _listData is DataGridListData)
	        {
	            newSelected = _data[DataGridListData(_listData).dataField];
	
	        }
	        else if (_listData)
	        {
	            if (selectedField)
	                newSelected = _data[selectedField];
	
	        }
	        else
	        {
	            newSelected = _data;
	        }
	
	        if (newSelected !== undefined && !selectedSet)
	        {
	            selected = newSelected as Boolean;
	            selectedSet = false;
	        }
	        
	        dispatchEvent(new FlexEvent(FlexEvent.DATA_CHANGE));
	    }
	    
	    
	    
	   
	    /**
	    * Storage for the listData property.
	    */
	    private var _listData:BaseListData;
	
	    [Bindable("dataChange")]
	    [Inspectable(environment="none")]
	    public function get listData():BaseListData
	    {
	        return _listData;
	    }
		public function set listData(value:BaseListData):void
	    {
	        _listData = value;
	    }
		
		
		/***
		* Some extensions of this class may be required to not have a background drawn,
		* so this provides and ability to specify that via the constructor
		* of the extending class.
		**/
		private var _backgroundEnabled:Boolean = true;
		public function set backgroundEnabled(value:Boolean):void
		{
			_backgroundEnabled = value;
			invalidateProperties();
	        invalidateDisplayList();
		}
		public function get backgroundEnabled():Boolean
		{
			return _backgroundEnabled;
		}

	    
	    override public function styleChanged(styleProp:String):void
	    {
	        styleChangedFlag = true;
	        super.styleChanged(styleProp);
	        invalidateProperties();
	       	invalidateDisplayList();
	    }
	    
	    private var _enableDrag:Boolean = true;
	    
	    public function get enableDrag() : Boolean
	    {
	    	return _enableDrag;
	    }
	    
	    public function set enableDrag(enblDrag:Boolean) : void
	    {
	    	_enableDrag = enblDrag;
	    }
	    
	    /**
	    * Events Handlers
	    **/
    	protected function rollOverHandler(event:MouseEvent):void
    	{
    		//trace(event);
    		
    	}
		
		protected function rollOutHandler(event:MouseEvent):void
    	{
    		//trace(event);
    	}
    	
    	protected function mouseDownHandler(event:MouseEvent):void
    	{
    		if(this._enableDrag) {
    			this.addEventListener(MouseEvent.MOUSE_MOVE,mouseMoveHandler);
    			this.startDrag();
    		}
    		//trace(event);
    	}
    	
    	protected function mouseMoveHandler(event:MouseEvent):void
    	{
    		//trace(event);
    	}
    	
    	protected function mouseUpHandler(event:MouseEvent):void
    	{
    		if(this._enableDrag) {
    			this.stopDrag();
    			this.removeEventListener(MouseEvent.MOUSE_MOVE,mouseMoveHandler);
    		}
    		//trace(event);
    	}
		
		protected function clickHandler(event:MouseEvent):void
    	{
    		selected = !selected;
    		//trace(event);
    	}		
		
		/**
		* Test to see if a filter exists. Will be Extended to support multiple filters.
		**/
		private function filterPosition(filterClass:Class):int {
        	for (var i:int = 0; i < filters.length; i++) {
            	if (filters[i] is filterClass) {
                	return i;
             	}
         	}
         	return -1;
     	}
     
		
		
	    /**
	    * End any fill or gradient fill drawing actions on this event
	    * so that the classes extending this one can do their specific 
	    * drawing to the surface.
	    **/
		private function handleUpdateComplete(event:FlexEvent):void
		{
			//End the fill always for each extension, since all common drawing is done in the 
			//base class. Including fills.
			if (backgroundEnabled)
			{
				graphics.endFill();
			}
		}
		
		override protected function updateDisplayList(unscaledWidth:Number,unscaledHeight:Number):void
		{
			
			super.updateDisplayList(unscaledWidth,unscaledHeight);
			
			//clear the drawing surface.
			graphics.clear();
			
			
			/**
			* Set up a drop shadow if required.
			**/
			if (getStyle("shadowEnabled"))
			{
				var dropShadowColor:uint= getStyle("shadowColor");
				var shadowDirection:Number= getStyle("shadowAngle");
				var shadowDistance:Number= getStyle("shadowDistance");
				var shadowAlpha:Number= getStyle("shadowAlpha");
				var shadowAngle:Number= getStyle("shadowAngle");
				
				if (dropShadowColor==0)
				{
					dropShadowColor=0x000000;
				}
				
				if (shadowDistance==0)
				{
					shadowDistance=30;
				}
				
				if (shadowAlpha==0)
				{
					shadowAlpha=1;
				}
				
				
				var tempFilter:DropShadowFilter = new DropShadowFilter(shadowDistance,shadowAngle,dropShadowColor,shadowAlpha);
    	    	var objectShadowFilter:Array = new Array();
	    	    objectShadowFilter.push(tempFilter);
	    	    	
	        	filters.splice(filterPosition(DropShadowFilter),1);
	        	if (filters.length !=0){
	        		filters.push(tempFilter);
	        	}
	        	else
	        	{
		        	filters=objectShadowFilter;
	        	}
	        }
	  		else
	  		{
	  			filters = null;
	  		}
	  		
			/**
			* Setup the line styles as required.
			**/
			//get the border color
			var borderColor:uint;
			if (!selected)
			{
				borderColor = getStyle("borderColor");
			}
			else
			{
				borderColor = getStyle("borderSelectedColor");
			}
			
			if (borderColor ==0)
			{
				borderColor = 0x000000;
			}
			
			var borderThickness:Number = getStyle("borderThickness");
			var borderAlpha:Number = getStyle("borderAlpha");
			var borderPixelHinting:Boolean = getStyle("borderPixelHinting");
			var borderJointStyle:String = getStyle("borderJointStyle");
	
			graphics.lineStyle(borderThickness,borderColor,borderAlpha,borderPixelHinting,"normal",null,borderJointStyle);
			
			if (getStyle("borderFillEnabled"))
			{
				var borderColors:Array
				if (!selected)
				{
					borderColors = getStyle("borderColors");
				}
				else
				{
					borderColors = getStyle("borderSelectedColors");
				}
				
				var borderAlphas:Array = getStyle("borderAlphas");
				var borderRatios:Array = getStyle("borderRatios");
				
				if (borderColors == null)
				{
					borderColors=[0x000000,0xFFFFFF];
				}
				
				if (borderAlphas == null)
				{
					borderAlphas=[1,1];
				}
				
				if (borderRatios == null)
				{
					borderRatios=[0,255];
				}
				
				var borderFillType:String = getStyle("borderFillType");
				var borderSpreadMethod:String = getStyle("borderSpreadMethod");
				var borderFocalPointRatio:Number = getStyle("borderFocalPointRatio");
				
				var borderAngle:Number = getStyle("borderAngle");
				
				if (!borderAngle)
				{
					borderAngle =0;
				}
								
				if (!borderFillType)
				{
					borderFillType = "linear";
				}
				
				if (!borderSpreadMethod)
				{
					borderSpreadMethod = "pad";
				}
												
				var borderMatrix:Matrix = new Matrix();
				borderMatrix.createGradientBox(unscaledWidth,unscaledHeight,borderAngle, 0, 0);
				graphics.lineGradientStyle(borderFillType, borderColors, borderAlphas, borderRatios, borderMatrix,borderSpreadMethod,"rgb",borderFocalPointRatio);
				
			}
					
			//If fillcolors is set then use a gradient.
			if (getStyle("fillEnabled") && backgroundEnabled)
			{
				
				var fillColors:Array;
				if (!selected){
					fillColors = getStyle("fillColors");
				}
				else{
					fillColors = getStyle("fillSelectedColors");
				}
				
				var fillAlphas:Array = getStyle("fillAlphas");
				var fillRatios:Array = getStyle("fillRatios");
				
				if (fillColors == null)
				{
					fillColors=[0x000000,0xFFFFFF];
				}
				
				if (fillAlphas == null)
				{
					fillAlphas=[1,1];
				}
				
				if (fillRatios == null)
				{
					fillRatios=[0,255];
				}
				
				var fillType:String = getStyle("fillType");
				var fillSpreadMethod:String = getStyle("fillSpreadMethod");
				var fillFocalPointRatio:Number = getStyle("fillFocalPointRatio");
				
				var fillAngle:Number = getStyle("fillAngle");
				
				if (!fillAngle)
				{
					fillAngle =0;
				}
								
				if (!fillType)
				{
					fillType = "linear";
				}
				
				if (!fillSpreadMethod)
				{
					fillSpreadMethod = "pad";
				}
												
				
				var fillMatrix:Matrix = new Matrix();
				fillMatrix.createGradientBox(unscaledWidth,unscaledHeight,fillAngle, 0, 0);
				graphics.beginGradientFill(fillType, fillColors, fillAlphas, fillRatios, fillMatrix,fillSpreadMethod,"rgb",fillFocalPointRatio);
				
			}
			else 
			{
				
				if (backgroundEnabled){
					
					var backgroundColor:uint; 
					if (!selected)
					{
						backgroundColor= getStyle("backgroundColor");
					}
					else
					{
						backgroundColor= getStyle("backgroundSelectedColor");
					}
					
					var backgroundAlpha:Number = getStyle("backgroundAlpha");
				
					if (backgroundColor !=0)
					{
						graphics.beginFill(backgroundColor,backgroundAlpha);	
					}
				}
				
			}
			
		}
		
		
	}
}