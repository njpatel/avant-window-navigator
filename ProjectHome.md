# \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ #
# ANNOUNCEMENT #

**This project has moved to [launchpad.net](https://launchpad.net/awn).
Instructions to get the new code is available at that link. Please report bugs/feature requests/translations to launchpad and not here, thanks!**

# \/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/ #


# About Avant #

Avant Window Navgator (Awn) is a dock-like bar which sits at the bottom of the screen (in all its composited-goodness) tracking open windows. I suppose this is a good time for a screen shot:

![http://avant-window-navigator.googlecode.com/svn/tags/web/bar-shot.png](http://avant-window-navigator.googlecode.com/svn/tags/web/bar-shot.png) ![http://avant-window-navigator.googlecode.com/svn/tags/web/text.png](http://avant-window-navigator.googlecode.com/svn/tags/web/text.png)

Awn uses libwnck to keep a track of open windows, and behaves exactly like a normal window-list :
  * Clicking an icon switches to that window, clicking again will minimise the window
  * Right-clicking will bring up a menu exactly like that of what you see on the window-list, allowing you to max, min, close, resize etc the window.
  * Dragging something on top of an icon will activate that window.
  * Visually (and quite attractively) responds to 'needs attention' & 'urgent' events
  * Can show windows from the entire viewport, or just the visible viewport.

Well, that's the the boring details over and done with, now on with the most important part, the looks:


![http://avant-window-navigator.googlecode.com/svn/tags/web/many-faces.png](http://avant-window-navigator.googlecode.com/svn/tags/web/many-faces.png)

The bar can be customised extensively:
  * Choose between two 'engines':
    * Glass engine. All the gradients can be changed to your tastes.
    * Pattern engine. That's right, you can now have a leopard print bar to match your underpants :).
  * The border & internal border colours can be changed.
  * The 'glow' (the background of an active window icon) can be changed, to be as simple or as lavish as you require.
  * Choose between sharp or rounded corners.
  * Choose the text & shadow colour of the window title text.

All preferences can be adjusted using the provided avant-preferences program, which provides a friendly interface to the gconf settings:


![http://avant-window-navigator.googlecode.com/svn/tags/web/prefs.png](http://avant-window-navigator.googlecode.com/svn/tags/web/prefs.png)

# Download #

Awn is currently under development, however you can try it out by either downloading the latest release from the [downloads page](http://code.google.com/p/avant-window-navigator/downloads/list). If, however, you live your life on the edge, you can always check out the latest code from the [subversion repository](http://code.google.com/p/avant-window-navigator/source). I promise to keep the svn code as break-free as possible, but do expect that sometimes it may not compile.

Awn is written in C and uses Gtk & GConf. Avant-preferences is written in python, and needs pygtk and the python bindings to gconf.

Awn needs a composited environment to run in, therefore you should be running either AIGLX or XGL with a compositor (such as Compiz or Beryl). It also depends on libwnck (libwnck-devel), and you will need all the usual development packages on your computer to install it.

**Update** SVN version no longer requires a patched libwnck! (It does however, need testers)

You will almost definatley need to build libwnck again due to one silly change you need to make in the source:

in libwnck/private.h change
```
#define DEFAULT_ICON_WIDTH          XX - this could be any number, most likely 32
#define DEFAULT_ICON_HEIGHT         XX
```
to
```
#define DEFAULT_ICON_WIDTH          48
#define DEFAULT_ICON_HEIGHT         48
```
you need to update & install libwnck with these changes **before** you install Awn.

I hope to get rid of this requirement by the next release, by getting the window icon from  Xorg directly.

# Planned Features #

Here's a list of features/items that I am currently working on:
  * ~~Try and window icons directly from Xorg, rather than through libwnck.~~ - fixed in svn, needs testers.
  * ~~Move to GObject code, especially for the icons (apps).~~
  * ~~Change icon rendering + animations to cairo (for smoother animations & more stability)~~
  * ~~Set-up Dbus backend to allow applications to have some control over their icon i.e. Album art for rhythmbox, progress bar for deluge/firefox, unread mail for evolution/thunderbird etc See mock-ups below.~~
  * ~~Allow the bar to auto hide.~~
  * Allow the bar to be vertical.
  * ~~Smart shortcuts (like Mac OS X dock), allow launching of programs & controling the launched programs window.~~
  * Allow the window manager to group windows i.e. group all the Gimp windows together, so they all raise when you click on the icon. (Would make glade2 development a bit easier ;-)
  * ~~Integrate better with window managers, like the window-list applet, therefore allowing the WM to shrink windows to the relevant icon, and enabling use of tasklist-based plugins.~~

Well, thats the list for now, I know i'll think of more things, and i'll be sure to cross off items as they are completed. If you have any ideas, please mail me at njpatel\_AT\_gmail\_DOT\_com.

# Get Involved #

## Create Packages ##
> Create packages for your distribution of choice (please include libwnck & libwnck-devel).

## Bugs & Features ##

File bugs & feature enhancements using the 'Issues' tab above, you will need a google.com account (gmail accounts work as well). If you do not have one, you can sign up for one, or you can send bugs directly to me : njpatel\_AT\_gmail\_DOT\_com.

## Source ##

The latest source is available from the 'Source' tab above. Also, you can browse the latest sources [online](http://avant-window-navigator.googlecode.com/svn/).

## Screenshots ##

**Update:** I have added some screenshots & a screencast to my [blog](http://njpatel.blogspot.com/).

I want to create a few pages of screen shots showing different looks that are possible with Awn, so if you think that your desktop is the bees knees, send me an email.