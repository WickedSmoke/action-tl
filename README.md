Action Timeline
===============

Action Timeline is a GUI to track character initiative and actions during
combat for role playing games.

![Screenshot](http://outguard.sourceforge.net/images/action-tl-0.6.png)

Here are the basic features provided:

  * Track intiative order.
  * Show actions on a second by second basis.
  * Roll dice to resolve actions.
  * Advance round by either 6 or 10 seconds.

There is currently no way to save data but some setup can be done using
command line arguments.


Managing Characters
-------------------

Characters can be added with the New Character button or naming them on
the command line.  They can be removed using the context menu "Delete" item.

To select a character left click on its row or scroll the mouse wheel to move
the selection up or down.

The order of characters can be changed using the Order Up/Down buttons or
holding **SHIFT** while scrolling the mouse wheel.


Managing Actions
----------------

Actions can be added to a character's timeline by either dragging them from
the action list or selecting them in the list.  List selection is done by
double-clicking the action or pressing return when the list has keyboard
focus.

Pressing the **Del** key will remove the last action of the currently selected
character.  Other actions can be Deleted using the context menu (right mouse
button) when the mouse pointer is over them.


Command Line Arguments
======================

Character names and actions can be provided on the command line.  Actions
are specified by a Name and Seconds duration separated by a colon.  If no
colon is present then the argument is treated as a character name.

Here's an example with four characters and two actions:

    ./action-tl Frederick Shana "Orc #1" "Orc #2" Toss:2 "Read Scroll:6"


How to Compile
==============

Qt 5 is required.  Project files are provided for QMake & m2.

To build with QMake:

    qmake-qt5; make

To build with m2:

    m2; make
