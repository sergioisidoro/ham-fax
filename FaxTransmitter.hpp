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

#ifndef FAXTRANSMITTER_HPP
#define FAXTRANSMITTER_HPP

#include <qobject.h>
#include "FaxImage.hpp"

class FaxTransmitter : public QObject {
	Q_OBJECT
public:
	FaxTransmitter(QObject* parent, FaxImage* faxImage);
	void startTransmission(void);
	void getValues(double* buffer, unsigned int& maxSamples);
private:
	FaxImage* faxImage;
	enum { APTSTART, PHASING, ENDPHASING, IMAGE, APTSTOP, IDLE } state;
	unsigned int sampleNr;
	unsigned int sampleRate;
	unsigned int lpm;
	unsigned int carrier;
	unsigned int deviation;
	unsigned int startLength;
	unsigned int startFreq;
	unsigned int phasingLines;
	bool phaseInvers;
	unsigned int stopLength;
	unsigned int stopFreq;
	bool color;
public slots:
        void setLPM(int lpm);
	void setAptStartFreq(int f);
	void setAptStartLength(int t);
	void setAptStopFreq(int f);
	void setAptStopLength(int t);
	void setPhasingLines(int n);
	void setPhasePol(bool pol);
	void setSampleRate(int rate);
	void setColor(bool b);
	void doNext(unsigned int n);
signals:
	void aptStart(void);
	void phasing(void);
	void imageLine(unsigned int n);
	void aptStop(void);
	void data(double* buf, unsigned int n);
};

#endif
