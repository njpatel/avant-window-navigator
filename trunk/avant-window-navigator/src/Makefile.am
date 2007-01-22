INCLUDES =					\
	$(AWN_CFLAGS)			\
	$(WARN_CFLAGS)

bin_PROGRAMS = affinity

AM_CPPFLAGS =								\
	-DDATADIR=\""$(datadir)"\"					\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

affinity_SOURCES =			\
	main.c				\
	awn-gconf.c			\
	awn-bar.c   			\
	awn-app.c      			\
	awn-win-manager.c    		\
	awn-title.c                     \
	awn-window.c	 		
                                        						

affinity_LDADD =			\
	$(AWN_LIBS)						
