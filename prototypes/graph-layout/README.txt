http://pippin.gimp.org/tmp/gegl-foo/
https://bugzilla.gnome.org/show_bug.cgi?id=465743
http://codecave.org/bauxite/index.html
http://codecave.org/gggl/index.html
http://pippin.gimp.org/tmp/gl/

The GraphLayout object manages the position of each child in a layout.
Idea: Create a Clutter-based graph that does not need user management of the layout.
All of its children would be handled internally, as well as click events and the likes.

From there, the user could create child nodes by clicking and choosing a node type.
The node properties would appear in an other region of the window, in some GTK+ widgets.

