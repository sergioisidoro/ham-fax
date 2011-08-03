## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = check-news dist-bzip2 nostdinc subdir-objects

bin_PROGRAMS = hamfax
hamfax_SOURCES = \
	src/Config.cpp src/Config.hpp\
        src/CorrectDialog.cpp src/CorrectDialog.hpp\
        src/DisplayLevel.cpp src/DisplayLevel.hpp\
        src/FaxImage.cpp src/FaxImage.hpp\
        src/FaxWindow.cpp src/FaxWindow.hpp\
        src/HelpDialog.cpp src/HelpDialog.hpp\
        src/ImageWidget.cpp src/ImageWidget.hpp\
        src/OptionsDialog.cpp src/OptionsDialog.hpp\
        src/ReceiveDialog.cpp src/ReceiveDialog.hpp\
        src/Spectrum.cpp src/Spectrum.hpp\
        src/TransmitDialog.cpp src/TransmitDialog.hpp\
	src/ToolTipFilter.cpp src/ToolTipFilter.hpp\
        \
        src/FaxDemodulator.cpp src/FaxDemodulator.hpp\
        src/FaxModulator.cpp src/FaxModulator.hpp\
        src/FaxReceiver.cpp src/FaxReceiver.hpp\
        src/FaxTransmitter.cpp src/FaxTransmitter.hpp\
        src/PTC.cpp src/PTC.hpp\
        src/File.cpp src/File.hpp\
        src/Sound.cpp src/Sound.hpp\
        src/PTT.cpp src/PTT.hpp\
        \
        src/Error.hpp src/Error.cpp\
        src/FirFilter.hpp\
        src/LookUpTable.hpp\
        src/hamfax.cpp

LIBS+=@Qt4_LIBS@
hamfax_CXXFLAGS = @Qt4_CFLAGS@ -Wall
hamfax_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\"

nodist_hamfax_SOURCES = \
	src/moc_Config.cpp\
	src/moc_CorrectDialog.cpp\
	src/moc_DisplayLevel.cpp\
	src/moc_FaxImage.cpp\
	src/moc_FaxWindow.cpp\
	src/moc_HelpDialog.cpp\
	src/moc_ImageWidget.cpp\
	src/moc_OptionsDialog.cpp\
	src/moc_ReceiveDialog.cpp\
	src/moc_Spectrum.cpp\
	src/moc_TransmitDialog.cpp\
	src/moc_FaxDemodulator.cpp\
	src/moc_FaxModulator.cpp\
	src/moc_FaxReceiver.cpp\
	src/moc_FaxTransmitter.cpp\
	src/moc_PTC.cpp\
	src/moc_File.cpp\
	src/moc_Sound.cpp\
	src/moc_ToolTipFilter.cpp\
	src/moc_PTT.cpp

moc_%.cpp: %.hpp
	moc $< -o $@

if USE_LRELEASE

TRANSLATION = po/hamfax_de.qm po/hamfax_fr.qm

# helper target for updating translation files
lupdate: $(hamfax_SOURCES)
	lupdate $(hamfax_SOURCES) -ts po/hamfax_de.ts
	lupdate $(hamfax_SOURCES) -ts po/hamfax_fr.ts

endif

pkgdata_DATA = doc/hamfax.html $(TRANSLATION)

EXTRA_DIST = po/hamfax_de.qm po/hamfax_de.ts po/hamfax_fr.qm po/hamfax_fr.ts \
	     doc/hamfax.html data/hamfax.desktop data/hamfax.svg

applicationsdir=$(datadir)/applications
applications_DATA=data/hamfax.desktop
iconsdir=$(datadir)/icons
icons_DATA=data/hamfax.svg
dist_man_MANS=doc/hamfax.1

.ts.qm:
	$(LRELEASE) $< -qm $@

# create ChangeLog file from git log
dist-hook: ChangeLog
	git log --pretty=short --abbrev-commit > $(distdir)/ChangeLog

## disabled until there is only one doc either html or xml
#.xml.html:
#	xmlto html-nochunks -o doc $<
#
#.xml.pdf:
#	xmlto pdf --extensions -o doc $<
#
#.xml.txt:
#	xmlto txt -o doc $<
