/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;

	/**
	 *  Event dispatched when an Entry has been added to the tag server.
	 **/
	public class AddEntryEvent extends Event
	{
		/** Event type constant; indicates that an Entry was added. */
		public static const ADD_ENTRY:String = "addEntry";
		
		/**
		 *  Constructor.
		 * 
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function AddEntryEvent(bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(ADD_ENTRY, bubbles, cancelable);
		}
		
		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new AddEntryEvent(bubbles,cancelable);
		}

	}
}