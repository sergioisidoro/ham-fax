// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
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

#ifndef FAXDEMODULATOR_HPP
#define FAXDEMODULATOR_HPP

#include <qobject.h>
#include "FirFilter.hpp"

class FaxDemodulator : public QObject {
	Q_OBJECT
public:
	FaxDemodulator(QObject* parent);
	~FaxDemodulator(void);
private:
	int carrier;
	int rate;
	int deviation;
	bool fm;
	static const int sine_size=8192;
	double* sine;
	double* sin_phase;
	double* cos_phase;
	FirFilter<double> iFir;
	FirFilter<double> qFir;
	double ifirold;
	double qfirold;
	static const int asine_size=256;
	double* asine;
signals:
	void data(int* buffer, int n);
public slots:
        void init(void);
        void setCarrier(int carrier);
	void setSampleRate(int sampleRate);
	void setDeviation(int dev);
	void setFM(bool fm);
	void newSamples(short* audio, int n);
	void setFilter(int n);
};

#endif
