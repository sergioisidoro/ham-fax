// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmittt, DH1CS <cschmit@suse.de>
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

#include <qwidget.h>
#include <qstring.h>
#include <qtimer.h>
#include <qmenubar.h>

#include "FaxControl.hpp"
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

class FaxWindow : public QWidget {
	Q_OBJECT
public:
	FaxWindow(const QString& version);
private:
	enum { FILE, DSP, SCSPTC };
	enum { SC288, SC576, R576288, R288576 };
	int interface;
	int pttID;
	void buildMenuBar(void);
	QString getFileName(QString caption, QString filter);
	virtual void closeEvent(QCloseEvent* close);
	QString version;
	QPopupMenu* optionsMenu;
	FaxTransmitter* faxTransmitter;
	FaxReceiver* faxReceiver;
	FaxImage* faxImage;
	FaxModulator* faxModulator;
	FaxDemodulator* faxDemodulator;
	FaxControl* faxControl;
	QMenuBar* menuBar;
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
	void scaleImage(int item);
	void quickSave(void);
};

#endif
