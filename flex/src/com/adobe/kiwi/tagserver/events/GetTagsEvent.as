/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.events
{
	import flash.events.*;

	/**
	 *  Event dispatched when all tags (for an account) are retrieved tag server.
	 **/
	public class GetTagsEvent extends Event
	{
		/** Event type constant; indicates that a set of tags was retrieved. */
		public static const GET_TAGS:String = "getTags";
		
		/**
		 *  Constructor.
		 * 
		 *  @param bubbles Determines whether the Event object participates in
		 *  the bubbling stage of the event flow.  The default value is false.
		 *  @param cancelable Determines whether the Event object can be canceled.
		 *  The default value is false.
		 **/
		public function GetTagsEvent(bubbles:Boolean=false,cancelable:Boolean=false):void
		{
			super(GET_TAGS);
		}

		/**
		 *  Duplicates this Event object.
		 *  @return the cloned Event object
		 **/
		public override function clone():Event
		{
			return new GetTagsEvent(bubbles,cancelable);
		}

		/** The retrieved tags. */
		public var tags:Object;
	}
}