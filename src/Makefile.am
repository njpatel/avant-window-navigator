INCLUDES =					\
	$(AWN_CFLAGS)			\
	$(WARN_CFLAGS)

bin_PROGRAMS = avant-window-navigator

AM_CPPFLAGS =								\
	-DDATADIR=\""$(datadir)"\"					\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

avant_window_navigator_SOURCES =			\
	main.c				\
	awn-gconf.h			\
	awn-gconf.c			\
	awn-bar.h			\
	awn-bar.c   			\
	awn-app.h			\
	awn-app.c      			\
	awn-dbus-glue.h			\
	awn-hotspot.h			\
	awn-hotspot.c			\
	awn-marshallers.h		\
	awn-marshallers.c		\
	awn-win-manager.h		\
	awn-win-manager.c    		\
	awn-title.h			\
	awn-title.c                     \
	awn-task.h			\
	awn-task.c			\
	awn-task-manager.h		\
	awn-task-manager.c		\
	awn-window.h			\
	awn-window.c	 		\
	awn-x.h				\
	awn-x.c				\
	awn-utils.h			\
	awn-utils.c			\
	xutils.h			\
	xutils.c			

icon_test_SOURCES=		\
	icon-test.c             \
	xutils.h		\
	xutils.c                           	
						
icon_test_LDADD =			\
	$(AWN_LIBS)

awn-dbus-glue.h: awn-dbus.xml Makefile
	$(LIBTOOL) --mode=execute $(DBUS_GLIB_BIN)/dbus-binding-tool --prefix=awn_task_manager --mode=glib-server --output=$@ $<


awn-marshallers.c: awn-marshallers.list
	glib-genmarshal --prefix _awn_marshal --body $< > $@

awn-marshallers.h: awn-marshallers.list
	glib-genmarshal --prefix _awn_marshal --header $< > $@

BUILT_SOURCES = awn-dbus-glue.h awn-marshallers.h awn-marshallers.c
EXTRA_DIST = awn-dbus.xml awn-marshallers.list

noinst_PROGRAMS=		\
	icon-test		

avant_window_navigator_LDADD =			\
	$(AWN_LIBS)						

