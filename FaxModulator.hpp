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

#ifndef FAXMODULATOR_HPP
#define FAXMODULATOR_HPP

#include <qobject.h>

class FaxModulator : public QObject {
	Q_OBJECT
public:
	FaxModulator(QObject* parent);
	~FaxModulator(void);
	void modulate(signed short* sample, double* buffer, unsigned int n);
	void setSampleRate(unsigned int sr);
private:
	static const unsigned int size_sine=8192;
	signed short* sine;        // lookup table for sine
	unsigned int phase;        // current phase of sine
	unsigned int sampleRate;
	bool fm;
	unsigned int carrier;
	unsigned int dev;
signals:
	void newCarrier(int carrier);
	void newDeviation(int dev);
	void newModulation(bool FM);
public slots:
        void setCarrier(int carrier);
	void setDeviation(int dev);
	void setFM(bool fm);
};

#endif
