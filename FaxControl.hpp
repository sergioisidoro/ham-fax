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

#ifndef FAXCONTROL_HPP
#define FAXCONTROL_HPP

#include <qspinbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qwidget.h>

class FaxControl : public QWidget {
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
	void newCarrier(unsigned int carrier);
	void newDeviation(unsigned int deviation);
	void newLPM(unsigned int lpm);
	void newModulation(bool fm);
	void newAptStartFreq(unsigned int f);
	void newAptStartLength(unsigned int t);
	void newAptStopFreq(unsigned int f);
	void newAptStopLength(unsigned int t);
	void newPhasingLength(unsigned int lines);
	void newPhasingPol(bool pol);
public slots:
	void setCarrier(unsigned int carrier);
	void setDeviation(unsigned int deviation);
	void setLPM(unsigned int lpm);
	void setModulation(bool fm);
	void setAptStartFreq(unsigned int f);
	void setAptStartLength(unsigned int t);
	void setAptStopFreq(unsigned int f);
	void setAptStopLength(unsigned int t);
	void setPhasingLines(unsigned int lines);
	void setPhasingPol(bool pol);
        void setImageSize(unsigned int width, unsigned int height);
private slots:
        void newModSelect(int index);
	void newPolSelect(int index);
	void newCarSelect(int carrier);
	void newDevSelect(int dev);
	void newLpmSelect(int lpm);
	void newAptStartFreqSel(int f);
	void newAptStartLengSel(int l);
	void newAptStopFreqSel(int f);
	void newAptStopLengSel(int l);
	void newPhaseLinesSel(int n);
};

#endif
