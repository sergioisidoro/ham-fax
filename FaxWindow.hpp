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

#include <qmainwindow.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qcombobox.h>

#include "Config.hpp"
#include "FaxDemodulator.hpp"
#include "FaxImage.hpp"
#include "FaxModulator.hpp"
#include "FaxReceiver.hpp"
#include "FaxTransmitter.hpp"
#include "File.hpp"
#include "PTC.hpp"
#include "PTT.hpp"
#include "ReceiveDialog.hpp"
#include "Sound.hpp"
#include "TransmitDialog.hpp"

class FaxWindow : public QMainWindow {
	Q_OBJECT
public:
	FaxWindow(const QString& version);
private:
	enum { FILE, DSP, SCSPTC };
	int interface;
	int pttID;
	int scrollID;
	void buildMenuBar(void);
	QString getFileName(QString caption, QString filter);
	virtual void closeEvent(QCloseEvent* close);
	QToolBar* modTool;
	QToolBar* aptTool;
	QToolBar* faxTool;
	QComboBox* modulation;
	QComboBox* invertPhase;
	QLabel* sizeText;
	QLabel* iocText;
	Config* config;
	QString version;
	QPopupMenu* optionsMenu;
	FaxTransmitter* faxTransmitter;
	FaxReceiver* faxReceiver;
	FaxImage* faxImage;
	FaxModulator* faxModulator;
	FaxDemodulator* faxDemodulator;
	Sound* sound;
	File* file;
	PTT* ptt;
	PTC* ptc;
	QTimer* timer;
	TransmitDialog* transmitDialog;
	ReceiveDialog* receiveDialog;
signals:
	void loadFile(QString fileName);
	void saveFile(QString fileName);
public slots:
        void setModulation(bool b);
	void setPhasingPol(bool b);
	void endTransmission(void);
	void endReception(void);
private slots:
        void load(void);
        void save(void);
	void initTransmit(int item);
	void transmitNext(void);
	void initReception(int item);
	void receiveNext(void);
        void about(void);
	void aboutQT(void);
	void doOptionsDialog(void);
	void doScaleDialog(void);
	void changePTT(void);
	void changeScroll(void);
	void setAutoScroll(bool b);
	void quickSave(void);
	void newImageSize(unsigned int w, unsigned int h);
};

#endif
