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

#include <qobject.h>

// these coefficents are taken from ACfax
const double lpf[]={0,-18,-38,-39,0,83,191,284,320,284,191,83,0,-39,-38,-18,0};

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
	static const int fir_size=17;
	double* ifir;
	double* ifir_end;
	double* icurrent;
	double ifirold;
	double* qfir;
	double* qfir_end;
	double* qcurrent;
	double qfirold;
	static const int asine_size=256;
	double* asine;
signals:
	void data(int* buffer, int n);
public slots:
        void setCarrier(int carrier);
	void setSampleRate(int sampleRate);
	void setDeviation(int dev);
	void setFM(bool fm);
	void newSamples(short* audio, int n);
};
