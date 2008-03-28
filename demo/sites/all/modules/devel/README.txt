README.txt
==========

A module containing helper functions for Drupal developers and
inquisitive admins. This module can print a log of
all database queries for each page request at the bottom of each page. The
summary includes how many times each query was executed on a page
(shouldn't run same query multiple times), and how long each query
 took (short is good - use cache for complex queries).

Also a dpr() function is provided, which pretty prints arrays and strings. Useful during
development.

Included in this package is also: 
- devel_node_access module which prints out the node_access records for a given node._
- macro.module which records form submissions and can pay them back later or on another site. More
information available at http://drupal.org/node/79900.


AUTHOR/MAINTAINER
======================
-moshe weitzman
weitzman at tejasa DOT com

NOTE
====
The subdirectory generate/ includes some scripts that can be used to help testing
on a Drupal site. See generate/README.txt for details.
