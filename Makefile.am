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
        src/OptionsDialog.cpp src/OptionsDialog.hpp\
        src/ReceiveDialog.cpp src/ReceiveDialog.hpp\
        src/Spectrum.cpp src/Spectrum.hpp\
        src/TransmitDialog.cpp src/TransmitDialog.hpp\
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
nodist_hamfax_SOURCES = \
	src/moc_Config.cpp\
	src/moc_CorrectDialog.cpp\
	src/moc_DisplayLevel.cpp\
	src/moc_FaxImage.cpp\
	src/moc_FaxWindow.cpp\
	src/moc_HelpDialog.cpp\
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
	src/moc_PTT.cpp

hamfax_LDFLAGS = @QT_LIBS@
hamfax_CXXFLAGS = @QT_CXXFLAGS@ -Wall

moc_%.cpp: %.hpp
	$(QT_MOC) $< -o $@

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\"

if USE_LRELEASE
TRANSLATION = po/hamfax_de.qm
endif

pkgdata_DATA = doc/HamFax.html $(TRANSLATION)

EXTRA_DIST = po/hamfax_de.qm po/hamfax_de.ts doc/HamFax.html

po/hamfax_de.qm: po/hamfax_de.ts
	$(LRELEASE) $< -qm $@

