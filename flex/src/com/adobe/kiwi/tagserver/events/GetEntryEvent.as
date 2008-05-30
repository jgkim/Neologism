/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;
	import com.adobe.kiwi.tagserver.model.*;
	
	/**
	 *  Event dispatched when a specific entry has been retrieved from the tag server.
	 **/
	public class GetEntryEvent extends Event
	{
		/** Event type constant; indicates that an entry was retrieved. */
		public static const GET_ENTRY:String = "getEntry";
		
		/**
		 *  Constructor.
		 * 
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function GetEntryEvent(bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(GET_ENTRY);
		}

		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new GetEntryEvent(bubbles,cancelable);
		}

		/** The retrieved entry. */
		public var entry:TagServerEntry;
	}
}