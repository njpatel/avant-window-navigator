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

schemadir   = /etc/gconf/schemas
schema_DATA = avant-window-navigator.schemas
@INTLTOOL_SCHEMAS_RULE@


$(schema_DATA).in: $(schema_DATA).in.in
	sed -e "s|\@PKGDATADIR\@|$(pkgdatadir)|g" $< > $@

if GCONF_SCHEMAS_INSTALL
install-data-local: $(schema_DATA)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA);
endif

EXTRA_DIST = $(schemas_DATA).in.in $(icon_DATA) $(desktop_DATA).in.in 

CLEANFILES = $(schemas_DATA) $(schemas_DATA).in $(desktop_DATA) $(desktop_DATA).in
