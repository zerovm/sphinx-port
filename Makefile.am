SUBDIRS = src antiword-0.37 catdoc-0.94.4 hypermail zxpdf-3.03 zsphinx/src test doc

EXTRA_DIST = api storage sphinx.conf.in sphinx-min.conf.in example.sql
sysconf_DATA = sphinx.conf.dist sphinx-min.conf.dist example.sql

install-data-hook:
	mkdir -p $(DESTDIR)$(localstatedir)/data && mkdir -p $(DESTDIR)$(localstatedir)/log
