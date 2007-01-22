schemasdir   = @GCONF_SCHEMA_FILE_DIR@
schemas_DATA = affinity.schemas
@INTLTOOL_SCHEMAS_RULE@


$(schemas_DATA).in: $(schemas_DATA).in.in
	sed -e "s|\@DATADIR\@|$(datadir)|g" $< > $@

install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
		done \
	fi

EXTRA_DIST = $(schemas_DATA).in.in 

CLEANFILES = $(schemas_DATA) $(schemas_DATA).in
