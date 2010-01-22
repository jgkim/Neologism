/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;
	import mx.rpc.*;

	/**
	 *  Event dispatched when an TagServer operation has failed.
	 **/
	public class TagServerFaultEvent extends Event
	{
		/** Event type constant; indicates that an TagServer operation failed. */
		public static const TAGSERVER__FAULT:String = "tagserverFault";
		
		/**
		 *  Constructor.
		 * 
		 *  @param fault The reason for the failure.
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function TagServerFaultEvent(fault:Fault=null,bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(TAGSERVER__FAULT, bubbles, cancelable);
			this.fault = fault;
		}
		
		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new TagServerFaultEvent(fault,bubbles,cancelable);
		}
		
		/** The reason for the failure. */
		public var fault:Fault;
	}
}