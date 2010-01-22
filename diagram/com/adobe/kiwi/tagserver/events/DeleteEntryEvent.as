/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;

	/**
	 *  Event dispatched when an Entry has been deleted from the tag server.
	 **/
	public class DeleteEntryEvent extends Event
	{
		/** Event type constant; indicates that an entry was deleted. */
		public static const DELETE_ENTRY:String = "deleteEntry";
		
		/**
		 *  Constructor.
		 * 
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function DeleteEntryEvent(bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(DELETE_ENTRY);
		}
		
		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new DeleteEntryEvent(bubbles,cancelable);
		}

	}
}