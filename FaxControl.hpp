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

#ifndef FAXCONTROL_HPP
#define FAXCONTROL_HPP

#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qvbox.h>

class FaxControl : public QVBox {
	Q_OBJECT
public:
	FaxControl(QWidget* parent);
private:
	QSpinBox* carrier;
	QSpinBox* dev;
	QSpinBox* lpm;
	QComboBox* modulation;
	QSpinBox* startFreq;
	QSpinBox* startLength;
	QSpinBox* stopFreq;
	QSpinBox* stopLength;
	QSpinBox* phasing;
	QComboBox* phasePol;
	QLabel* ioc;
	QLabel* width;
	QLabel* height;
signals:
	void newCarrier(int carrier);
	void newDeviation(int deviation);
	void newLPM(int lpm);
	void newModulation(bool fm);
	void newAptStartFreq(int f);
	void newAptStartLength(int t);
	void newAptStopFreq(int f);
	void newAptStopLength(int t);
	void newPhasingLength(int lines);
	void newPhasingPol(bool pol);
public slots:
	void setCarrier(int carrier);
	void setDeviation(int deviation);
	void setLPM(int lpm);
	void setModulation(bool fm);
	void setAptStartFreq(int f);
	void setAptStartLength(int t);
	void setAptStopFreq(int f);
	void setAptStopLength(int t);
	void setPhasingLines(int lines);
	void setPhasingPol(bool pol);
        void setImageSize(unsigned int width, unsigned int height);
private slots:
        void newModSelect(int index);
	void newPolSelect(int index);
};

#endif
