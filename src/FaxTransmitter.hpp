// HamFax -- an application for sending and receiving amateur radio facsimiles
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

#ifndef FAXTRANSMITTER_HPP
#define FAXTRANSMITTER_HPP

#include <qobject.h>
#include "FaxImage.hpp"

class FaxTransmitter : public QObject {
	Q_OBJECT
public:
	FaxTransmitter(QObject* parent, FaxImage* faxImage);
	void startTransmission(void);
private:
	FaxImage* image;
	enum { APTSTART, PHASING, ENDPHASING, IMAGE, APTSTOP, IDLE } state;
	int sampleNr;
	int sampleRate;
	int lpm;
	int carrier;
	int deviation;
	int startLength;
	int row;
	int startFreq;
	int phasingLines;
	bool phaseInvers;
	int stopLength;
	int stopFreq;
	bool color;
	int cols;
	int rows;
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
	void setImageSize(int cols, int rows);
	void doNext(int n);
	void doAptStop(void);
signals:
	void aptStart(void);
	void phasing(void);
	void imageLine(int n);
	void aptStop(void);
	void data(double* buf, int n);
	void start(void);
	void end(void);
};

#endif
