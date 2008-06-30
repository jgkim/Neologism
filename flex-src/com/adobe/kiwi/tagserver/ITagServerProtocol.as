/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver
{
	import com.adobe.kiwi.connections.*;
	import com.adobe.kiwi.tagserver.model.*;
	import com.adobe.net.*;
	
	import flash.events.*;
	
	import mx.collections.*;
	 	
	/**
	 *  Dispatched when a set of tags has been retrieved.
	 * 
	 *  @eventType com.adobe.kiwi.tagserver.events.GetTagsEvent
	 **/
	[Event(name="getTags", type="com.adobe.kiwi.tagserver.events.GetTagsEvent")]
	 	
	/**
	 *  Dispatched when a set of entries has been retrieved.
	 * 
	 *  @eventType com.adobe.kiwi.tagserver.events.GetEntriesEvent
	 **/
	[Event(name="getEntries", type="com.adobe.kiwi.tagserver.events.GetEntriesEvent")]
	 	
	/**
	 *  Dispatched when a specific entry has been retrieved.
	 * 
	 *  @eventType com.adobe.kiwi.tagserver.events.GetEntryEvent
	 **/
	[Event(name="getEntry", type="com.adobe.kiwi.tagserver.events.GetEntryEvent")]
	 	
	/**
	 *  Dispatched after an entry has been successfully added
	 * 
	 *  @eventType com.adobe.kiwi.tagserver.events.AddEntryEvent
	 **/
	[Event(name="addEntry", type="com.adobe.kiwi.tagserver.events.AddEntryEvent")]
	 	
	/**
	 *  Dispatched after an entry has been successfully deleted
	 * 
	 *  @eventType com.adobe.kiwi.tagserver.events.DeleteEntryEvent
	 **/
	[Event(name="deleteEntry", type="com.adobe.kiwi.tagserver.events.DeleteEntryEvent")]
	
	/**
	 * Interface for implementations of a tag server protocol similar to del.icio.us API.
	 * 
	 *  @see http://del.icio.us/help/api/
	 **/
	public interface ITagServerProtocol extends IProtocol, IEventDispatcher
	{
		/**
		 *  Retrieves all tags for the acount on the tag server.
		 * 
		 *  @event getTags GetTagsEvent Dispatched when the tags has been retrieved.
		 *  @event tagServerFault TagServerFaultEvent Dispatched when an operation generates a Fault.
		 **/
		function getTags():void;
		
		/**
		 *  Retrieves all entries that are tagged with the given list
		 * 
		 *  @param byTags the List of tag words to match
		 * 
		 *  @event getEntries GetEntriesEvent Dispatched when the entries has been retrieved.
		 *  @event tagServerFault TagServerFaultEvent Dispatched when an operation generates a Fault.
		 **/
		function getEntries(byTags:IList):void;
		
		/**
		 *  Retrieves a tag server entry that corresponds with a given URL
		 * 
		 *  @param byURL the URI of the entry to return
		 * 
		 *  @event getEntry GetEntryEvent Dispatched when the entry has been retrieved.
		 *  @event tagServerFault TagServerFaultEvent Dispatched when an operation generates a Fault.
		 **/
		function getEntry(byURL:URI):void;
		
		/**
		 *  Add a tag server entry to the tag server
		 * 
		 *  @param anEntry the tag server entry to add
		 * 
		 *  @event addEntry AddEntryEvent Dispatched when the entry has been successfully added.
		 *  @event tagServerFault TagServerFaultEvent Dispatched when an operation generates a Fault.
		 **/
		function addEntry(anEntry:TagServerEntry):void;
		
		/**
		 *  Remove a tag server entry that corresponds with a given URL
		 * 
		 *  @param byURL the URI of the entry to remove
		 * 
		 *  @event deleteEntry DeleteEntryEvent Dispatched when the entry has been successfully deleted.
		 *  @event tagServerFault TagServerFaultEvent Dispatched when an operation generates a Fault.
		 **/
		function deleteEntry(byURL:URI):void;
	}
}