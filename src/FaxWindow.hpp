// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001,2002
// Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
//  
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef FAXWINDOW_HPP
#define FAXWINDOW_HPP

#include <qcombobox.h>
#include <qmainwindow.h>
#include <qstring.h>
#include <qtoolbar.h>
#include <QAction>
#include <QMenu>
#include <QMainWindow>
#include "CorrectDialog.hpp"
#include "FaxDemodulator.hpp"
#include "FaxImage.hpp"
#include "FaxModulator.hpp"
#include "FaxReceiver.hpp"
#include "FaxTransmitter.hpp"
#include "File.hpp"
#include "PTC.hpp"
#include "ReceiveDialog.hpp"
#include "Sound.hpp"
#include "ToolTipFilter.hpp"
#include "TransmitDialog.hpp"

class FaxWindow : public QMainWindow {
	Q_OBJECT
public:
	FaxWindow(const QString& version);
private:
	// menus
	void createMenubar();
	QAction* slantAction;
	QAction* colorDrawAction;
	QAction* monoDrawAction;

	// tool bars
	void createToolbars();
	QToolBar* modTool;
	QComboBox* modulation;
	QComboBox* filter;
	QToolBar* aptTool;
	QToolBar* faxTool;
	QComboBox* invertPhase;
	QComboBox* colorBox;

	// status bar
	void createStatusBar();
	QLabel* sizeText;
	QLabel* iocText;

	// child objects
	File* file;
	FaxDemodulator* faxDemodulator;
	FaxModulator* faxModulator;
	FaxImage* faxImage;
	FaxReceiver* faxReceiver;
	FaxTransmitter* faxTransmitter;
	PTC* ptc;
	Sound* sound;
	CorrectDialog* correctDialog;
	TransmitDialog* transmitDialog;
	ReceiveDialog* receiveDialog;
	ToolTipFilter* toolTipFilter;

	int ioc;
	virtual void closeEvent(QCloseEvent* close);
	enum { FILE, DSP, SCSPTC };
	int interface;
	enum { WAITFIRST, WAITSECOND, NOTHING } slantState;

	void initTransmitCommon(int interface, int sampleRate);
	void initReceptionCommon(int interface, int sampleRate);
public slots:
        // part of set begin of line
	void setBeginEnd(void);

	// set if menu items are available
	void setImageAdjust(bool b);

	void endReception(void);
	void endTransmission(void);
	void slantEnd(void);
	void slantWaitSecond(void);
	void enableControls(void);
	void disableControls(void);
private slots:
	// from FaxImage
	void newImageSize(int w, int h);

	// slots for menu items
        // File
        void load(void);
        void save(void);
	void quickSave(void);

	// Transmit and Receive
	void initTransmitFile(void);
	void initTransmitDsp(void);
	void initTransmitPtc(void);
	void initReceptionFile(void);
	void initReceptionDsp(void);
	void initReceptionPtc(void);

	// Image
        void adjustIOC(void);
        void scaleToIOC(void);
	void slantWaitFirst(void);
	void redrawColor(void);
	void redrawMono(void);
	// shiftColors is in FaxImage
	void setBegin(void);

	// Options
	void doOptions(void);
	void selectFont(void);
	void changePTT(bool b);
	void changeScroll(bool b);
	void changeToolTip(bool b);

	// Help
	void help(void);
        void about(void);
	void aboutQT(void);

	// slots for toolbar objects
	// modulation
	void setCarrier(int c);
	void setDeviation(int d);
	void setUseFM(int f);
	void setFilter(int n);

	// apt
	void setAptStartLength(int l);
	void setAptStartFreq(int f);
	void setAptStopLength(int l);
	void setAptStopFreq(int f);

	// facsimile
	void setLpm(int l);
	void setPhaseLines(int l);
	void setPhaseInvert(int i);
	void setColor(int c);
};

#endif
