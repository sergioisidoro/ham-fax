// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <cschmit@suse.de>
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
#include "Config.hpp"
#include "CorrectDialog.hpp"
#include "FaxDemodulator.hpp"
#include "FaxImage.hpp"
#include "FaxModulator.hpp"
#include "FaxReceiver.hpp"
#include "FaxTransmitter.hpp"
#include "File.hpp"
#include "PTC.hpp"
#include "Sound.hpp"

class FaxWindow : public QMainWindow {
	Q_OBJECT
public:
	FaxWindow(const QString& version);
private:
	QString getFileName(QString caption, QString filter);
	virtual void closeEvent(QCloseEvent* close);
	QString version;
	enum { FILE, DSP, SCSPTC };
	int interface;
	QPopupMenu* imageMenu;
	int slantID;
	int colDrawID;
	int monoDrawID;
	int shift1ID;
	int shift2ID;
	QPopupMenu* optionsMenu;
	int pttID;
	int scrollID;
	int toolTipID;
	QToolBar* modTool;
	QComboBox* modulation;
	QToolBar* aptTool;
	QToolBar* faxTool;
	QComboBox* invertPhase;
	QComboBox* color;
	QLabel* sizeText;
	QLabel* iocText;
	Config* config;
	File* file;
	FaxDemodulator* faxDemodulator;
	FaxModulator* faxModulator;
	FaxImage* faxImage;
	FaxReceiver* faxReceiver;
	FaxTransmitter* faxTransmitter;
	PTC* ptc;
	Sound* sound;
	CorrectDialog* correctDialog;
	enum { WAITFIRST, WAITSECOND, NOTHING } slantState;
signals:
	void correctBegin(void);
	void correctSlant(void);
	void imageWidth(int w);
	void loadFile(QString fileName);
	void saveFile(QString fileName);
public slots:
	void setBeginEnd(void);
	void setColor(bool b);
	void setImageAdjust(bool b);
        void setModulation(bool b);
	void setPhasingPol(bool b);
	void endReception(void);
	void endTransmission(void);
	void slantEnd(void);
	void slantWaitSecond(void);
	void enableControls(void);
	void disableControls(void);
private slots:

        void load(void);
        void save(void);
	void quickSave(void);
	void initTransmit(int item);
	void initReception(int item);
	void doScaleDialog(void);
	void redrawColor(void);
	void redrawMono(void);
	void setBegin(void);
	void slantWaitFirst(void);
	void doOptionsDialog(void);
	void changePTT(void);
	void changeScroll(void);
	void setAutoScroll(bool b);
	void changeToolTip(void);
	void setToolTip(bool b);
	void help(void);
        void about(void);
	void aboutQT(void);
	void newImageSize(int w, int h);
};

#endif
