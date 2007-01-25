glade_DATA =  \
	   window.glade
gladedir = $(pkgdatadir)/


install-exec-local:
	sed -e "s|\@PKGDATADIR\@|$(pkgdatadir)|g" avant-preferences.in.py > avant-preferences.py
	$(INSTALL) -m 755 avant-preferences.py $(DESTDIR)$(bindir)/avant-preferences

EXTRA_DIST = $(glade_DATA) \
	avant-preferences.in.py

clean:
	rm -f *.pyo *.pyc *.bak *.gladep
