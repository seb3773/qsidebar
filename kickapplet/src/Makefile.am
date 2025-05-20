INCLUDES = $(all_includes)
METASOURCES = AUTO

# Définir le module KDE (TDE dans Trinity)
kde_module_LTLIBRARIES = qsidebar_applet_panelapplet.la

# Sources de l'applet, incluant les fichiers DCOP générés
qsidebar_applet_panelapplet_la_SOURCES = qsidebar_applet.cpp qsidebar_applet.skel qsidebar_applet.stub

# Dépendances pour générer les fichiers DCOP
qsidebar_applet_DCOPSKEL = qsidebar_applet.h
qsidebar_applet_DCOPSTUB = qsidebar_applet.h

# Options de linkage
qsidebar_applet_panelapplet_la_LDFLAGS = -module -avoid-version $(all_libraries) $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI) $(LIB_TDEPARTS)
qsidebar_applet_panelapplet_la_LIBADD = $(LIB_TDEUI)

# Installation du fichier .desktop
qsidebar_applet_DATA = qsidebar_applet.desktop
qsidebar_appletdir = $(kde_datadir)/kicker/applets

# Extraction des messages pour la traduction
messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` > rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/qsidebar_applet.pot