// $Id: README.txt,v 1.8.2.2 2007/01/05 23:33:23 yched Exp $

Content Construction Kit
------------------------
To install, place the entire cck folder into your modules directory.
Go to administer -> site building -> modules and enable the content module and one or
more field type modules:

- text.module
- number.module
- userreference.module
- nodereference.module

Now go to administer -> content management -> content types. Create a new
content type and edit it to add some fields. Then test by creating
a new node of your new type using the create content menu link.

The included optionswidget.module provides radio and check box selectors
for text and numeric types.

The included fieldgroup.module allows you to group fields together
in fieldsets to help organize them.

A comprehensive guide to using CCK is available as a CCK Handbook
at http://drupal.org/node/101723.

jvandyk [at] iastate.edu
jchaffer [at] structureinteractive.com