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

#ifndef RECEIVEDIALOG_HPP
#define RECEIVEDIALOG_HPP

#include <qdialog.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "DisplayLevel.hpp"
#include "Spectrum.hpp"

class ReceiveDialog : public QDialog {
	Q_OBJECT
public:
	ReceiveDialog(QWidget* parent);
private:
	virtual void closeEvent(QCloseEvent* close);
        void showText(const QString& s);
	QLabel* status;
	QLabel* aptText;
	QPushButton* skip;
	QPushButton* cancel;
	DisplayLevel* level;
	Spectrum* spectrum;
signals:
        void cancelClicked(void);
	void skipClicked(void);
public slots:
	void apt(int f);
	void aptStart(void);
	void phasing(void);
	void phasingLine(double lpm);
	void imageRow(int row);
	void disableSkip(void);
	void imageData(int* buffer, int n);
	void samples(short* buffer, int n);
};

#endif
