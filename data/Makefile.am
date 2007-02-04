SUBDIRS = active

desktopdir   = $(datadir)/applications
desktop_DATA = avant-window-navigator.desktop
@INTLTOOL_DESKTOP_RULE@


$(desktop_DATA).in: $(desktop_DATA).in.in
	sed -e "s|\@PKGDATADIR\@|$(pkgdatadir)|g" $< > $@
	
icon_DATA =  \
	avant-window-navigator.svg	\
	avant-window-navigator-24.png	\
	avant-window-navigator-48.png

icondir = $(pkgdatadir)

schemasdir   = /etc/gconf/schemas
schemas_in_files = avant-window-navigator.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

if GCONF_SCHEMAS_INSTALL
install-data-local: $(schema_DATA)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schemas_DATA);
endif


EXTRA_DIST = $(schemas_in_files) $(icon_DATA) $(desktop_DATA)

CLEANFILES = $(schemas_DATA) $(schemas_DATA)
