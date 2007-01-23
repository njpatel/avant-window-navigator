glade_DATA =  \
	   window.glade
gladedir = $(pkgdatadir)/

install-exec-local:
	$(INSTALL) -m 755 avant-preferences.py $(DESTDIR)$(bindir)/avant-preferences

EXTRA_DIST = $(glade_DATA) \
	avant-preferences.py

clean:
	rm -f *.pyo *.pyc *.bak *.gladep
