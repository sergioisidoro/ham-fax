## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = check-news dist-bzip2 nostdinc subdir-objects

bin_PROGRAMS = hamfax
hamfax_SOURCES = \
	src/Config.cpp src/Config.hpp src/moc_Config.cpp\
        src/CorrectDialog.cpp src/CorrectDialog.hpp src/moc_CorrectDialog.cpp\
        src/DisplayLevel.cpp src/DisplayLevel.hpp src/moc_DisplayLevel.cpp\
        src/FaxImage.cpp src/FaxImage.hpp src/moc_FaxImage.cpp\
        src/FaxWindow.cpp src/FaxWindow.hpp src/moc_FaxWindow.cpp\
        src/HelpDialog.cpp src/HelpDialog.hpp src/moc_HelpDialog.cpp\
        src/OptionsDialog.cpp src/OptionsDialog.hpp src/moc_OptionsDialog.cpp\
        src/ReceiveDialog.cpp src/ReceiveDialog.hpp src/moc_ReceiveDialog.cpp\
        src/Spectrum.cpp src/Spectrum.hpp src/moc_Spectrum.cpp\
        src/TransmitDialog.cpp src/TransmitDialog.hpp src/moc_TransmitDialog.cpp\
        \
        src/FaxDemodulator.cpp src/FaxDemodulator.hpp src/moc_FaxDemodulator.cpp\
        src/FaxModulator.cpp src/FaxModulator.hpp src/moc_FaxModulator.cpp\
        src/FaxReceiver.cpp src/FaxReceiver.hpp src/moc_FaxReceiver.cpp\
        src/FaxTransmitter.cpp src/FaxTransmitter.hpp src/moc_FaxTransmitter.cpp\
        src/PTC.cpp src/PTC.hpp src/moc_PTC.cpp\
        src/File.cpp src/File.hpp src/moc_File.cpp\
        src/Sound.cpp src/Sound.hpp src/moc_Sound.cpp\
        src/PTT.cpp src/PTT.hpp src/moc_PTT.cpp\
        \
        src/Error.hpp src/Error.cpp\
        src/FirFilter.hpp\
        src/LookUpTable.hpp\
        src/hamfax.cpp

hamfax_LDFLAGS = @QT_LIBS@
hamfax_CXXFLAGS = @QT_CXXFLAGS@

moc_%.cpp: %.hpp
	$(QT_MOC) $< -o $@

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\"

if USE_LRELEASE
TRANSLATION = po/hamfax_de.qm
endif

pkgdata_DATA = doc/HamFax.html $(TRANSLATION)

EXTRA_DIST = po/hamfax_de.qm

po/hamfax_de.qm: po/hamfax_de.ts
	cd po/; $(LRELEASE) hamfax.pro
