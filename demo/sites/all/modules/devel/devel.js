// $Id: devel.js,v 1.1.2.2 2007/10/21 01:06:35 weitzman Exp $

/**
  *  @name    jQuery Logging plugin
  *  @author  Dominic Mitchell
  *  @url     http://happygiraffe.net/blog/archives/2007/09/26/jquery-logging
  */
jQuery.fn.log = function (msg) {
    console.log("%s: %o", msg, this);
    return this;
};
