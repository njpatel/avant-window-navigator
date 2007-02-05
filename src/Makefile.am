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
	xutils.h			\
	xutils.c			

icon_test_SOURCES=		\
	icon-test.c             \
	xutils.h		\
	xutils.c                           	
						
icon_test_LDADD =			\
	$(AWN_LIBS)

noinst_PROGRAMS=		\
	icon-test		

avant_window_navigator_LDADD =			\
	$(AWN_LIBS)						

