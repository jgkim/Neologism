/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;

	/**
	 *  Event dispatched when a set of entries has been retrieved from the tag server.
	 **/
	public class GetEntriesEvent extends Event
	{
		/** Event type constant; indicates that set of entries was retreived. */
		public static const GET_ENTRIES:String = "getEntries";
		
		/**
		 *  Constructor.
		 * 
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function GetEntriesEvent(bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(GET_ENTRIES);
		}
	
		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new GetEntriesEvent(bubbles,cancelable);
		}

		/** The retrieved entries. */
		public var entries:Array;
	}
}