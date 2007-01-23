INCLUDES =					\
	$(AWN_CFLAGS)			\
	$(WARN_CFLAGS)

bin_PROGRAMS = avant-window-navigator

AM_CPPFLAGS =								\
	-DDATADIR=\""$(datadir)"\"					\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

avant_window_navigator_SOURCES =			\
	main.c				\
	awn-gconf.c			\
	awn-bar.c   			\
	awn-app.c      			\
	awn-win-manager.c    		\
	awn-title.c                     \
	awn-window.c	 		
                                        						

avant_window_navigator_LDADD =			\
	$(AWN_LIBS)						
