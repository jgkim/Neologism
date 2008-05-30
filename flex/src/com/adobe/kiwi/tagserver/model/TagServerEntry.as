/**
 * $id:kiwi_001

 * Copyright(c) 2006 Adobe Systems Incorporated. All Rights Reserved.
 */
package com.adobe.kiwi.tagserver.model
{
	import com.adobe.net.*;
	
	import mx.collections.*;
	
	/**
	 *  Class representing a tagserver entry, which fundamentally associates a URI with a list of tags
	 **/
	[Bindable] public class TagServerEntry
	{
		/** Constructor.
		 * 
		 *  @param url the URI of the tagserver entry
		 *  @param title the title of the tagserver entry
 * 		 *  @param description the description of the tagserver entry
 * 		 *  @param tags the tags of the tagserver entry
 * 		 *  @param shared indicates whether the tagserver entry is public or private
 * 		 *  @param timestamp date and time of the tagserver entry
 * 		 **/
		public function TagServerEntry(url:URI=null,
										title:String="",
										description:String="",
										tags:IList=null,
										shared:Boolean=true,
										timestamp:Date=null):void
		{
			this.url = url;
			this.title = title;
			this.description = description;
			this.tags = (tags != null ? tags : new ArrayCollection(new Array()));
			this.shared = shared;
			this.timestamp = timestamp;
		}
		
		/**
		 *  The URI of the tagserver entry.
		 **/
		public var url:URI;
		/**
		 *  The title of the tagserver entry.
		 **/
		public var title:String;
		/**
		 *  The description of the tagserver entry.
		 **/
		public var description:String;
		/**
		 *  The tags of the tagserver entry.
		 **/
		public var tags:IList;
		/**
		 *  Whether the tagserver entry is public or private.
		 **/
		public var shared:Boolean;
		/**
		 *  The date and time of the tagserver entry.
		 **/
		public var timestamp:Date;	
	}
}