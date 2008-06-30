/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.impl
{
	import com.adobe.kiwi.connections.*;
	import com.adobe.kiwi.tagserver.*;
	import com.adobe.kiwi.tagserver.events.*;
	import com.adobe.kiwi.tagserver.model.*;
	import com.adobe.net.*;
	import com.adobe.utils.*;
	
	import flash.events.*;
	
	import mx.collections.*;
	import mx.rpc.events.*;
	import mx.rpc.http.*;
	
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
	 * Contains a partial implementation of the tag server protocol based on the del.icio.us API.
	 **/
	public class TagServerProtocol extends EventDispatcher implements ITagServerProtocol
	{	
		/** Constructor.
		 * 
		 *  @param endpoint the URI of the tag server API endpoint
		 *  @param servicefactory factory class that generates transports the API calls
		 **/
		public function TagServerProtocol(endpoint:String,serviceFactory:IServiceFactory=null):void
		{
			this.endpoint = endpoint;
			this.serviceFactory = serviceFactory;
		}
		
		private function createService():HTTPService
		{
			var theService:HTTPService = null;
			
			if (serviceFactory != null)
			{
				theService = serviceFactory.createHTTPService();
			}
			else
			{
				theService = new HTTPService();
			}
			
			return theService;
		}
		
		
		private function sendRequest(url:URI, onResult:Function, onFault:Function, resultFormat:String=""):void
		{
			var request:HTTPService = createService();
			
			request.url = url.toString();

			if (resultFormat.length > 0)
				request.resultFormat = resultFormat;

			request.addEventListener("result",onResult);
			request.addEventListener("fault",onFault);
			
			request.send();
		}

		/**
		 *  @inheritDoc
		 **/
		public function getTags():void
		{
			var theURL:URI = new URI(endpoint + "/tags/get");
			sendRequest(theURL, onGetTagsResult, onFault);
		}

		private function onGetTagsResult(event:ResultEvent):void
		{
			var tagserverEvent:GetTagsEvent = new GetTagsEvent();
			tagserverEvent.tags = event.result.tags;
			dispatchEvent(tagserverEvent);
		}
		
		/**
		 *  @inheritDoc
		 **/
		public function getEntries(byTags:IList):void
		{
			var theURL:URI = new URI(endpoint + "/posts/all");
			theURL.setQueryValue("tag", byTags[0]);

			// Use a closure for the event handler so that it has access to
			// the filtering tags.
			var onGetEntriesResult:Function = function(event:ResultEvent):void
			{
				var entries:Array = new Array();
				var xml:XML = new XML(event.result);
				var posts:Array = new Array();
				var postXMLList:XMLList = xml.post;
	
				for each (var postXML:XML in postXMLList)
				{
					var missing:Boolean = false;
					for each (var tag:String in byTags)
					{
						var theTags:String = postXML.@tag.toString();
						if (theTags.search(tag) == -1)
						{
							missing = true;
							break;
						}
					}
					
					if (!missing)
					{
						// Verify that each tag in the request is in the post.
						var entry:TagServerEntry = new TagServerEntry(new URI(postXML.@href),
									postXML.@description,
									"",
									new ArrayCollection(String(postXML.@tag).split(" ")),
									true,
									DateUtil.parseW3CDTF(postXML.@time));
					
						entries.push(entry);
					}
				}
				
				var tagserverEvent:GetEntriesEvent = new GetEntriesEvent();
				tagserverEvent.entries = entries;
				dispatchEvent(tagserverEvent);
			}
			sendRequest(theURL, onGetEntriesResult, onFault, "xml");
		}

		/**
		 *  @inheritDoc
		 **/
		public function getEntry(byURL:URI):void
		{
			var theURL:URI = new URI(endpoint + "/posts/get");

			theURL.setQueryValue("url", byURL.toString());

			sendRequest(theURL, onGetEntryResult, onFault, "xml");
		}

		private function onGetEntryResult(event:ResultEvent):void
		{
			var xml:XML = new XML(event.result);
			var postXMLList:XMLList = xml.post;
			var anEntry:TagServerEntry = null;
			
			if (postXMLList.length() == 1)
			{
				anEntry = new TagServerEntry(new URI(postXMLList[0].@href),
							postXMLList[0].@description,
							"",
							new ArrayCollection(String(postXMLList[0].@tag).split(" ")));
			}
			
			var tagserverEvent:GetEntryEvent = new GetEntryEvent();
			tagserverEvent.entry = anEntry;
			dispatchEvent(tagserverEvent);
		}
		
		/**
		 *  @inheritDoc
		 **/
		public function addEntry(anEntry:TagServerEntry):void
		{
			var theURL:URI = new URI(endpoint + "/posts/add");
			
			theURL.setQueryValue("description", anEntry.title);
			theURL.setQueryValue("url", anEntry.url.toString());
			theURL.setQueryValue("tags", anEntry.tags.toArray().join(" "));
			theURL.setQueryValue("shared", anEntry.shared ? "yes" : "no");
			theURL.setQueryValue("replace", "yes");
			
			sendRequest(theURL, onAddEntryResult, onFault);
		}
		
		private function onAddEntryResult(event:ResultEvent):void
		{
			var tagserverEvent:AddEntryEvent = new AddEntryEvent();
			dispatchEvent(tagserverEvent);
		}
		
		/**
		 *  @inheritDoc
		 **/
		public function deleteEntry(byURL:URI):void
		{
			var theURL:URI = new URI(endpoint + "/posts/delete");

			theURL.setQueryValue("url", byURL.toString());

			sendRequest(theURL, onDeleteEntryResult, onFault);
		}
		
		private function onDeleteEntryResult(event:ResultEvent):void
		{
			var tagserverEvent:DeleteEntryEvent = new DeleteEntryEvent();;
			dispatchEvent(tagserverEvent);
		}
		
		private function onFault(event:FaultEvent):void
		{
			var tagserverEvent:TagServerFaultEvent = new TagServerFaultEvent();
			tagserverEvent.fault = event.fault;
			dispatchEvent(tagserverEvent);
		}
		
		private var endpoint:String;
		private var serviceFactory:IServiceFactory;
	}
}
