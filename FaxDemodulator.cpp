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

#include "FaxDemodulator.hpp"
#include <math.h>

// Narrow, middle and wide fir low pass filter from ACfax
static const double lpf[3][17]={
	{ -7,-18,-15, 11, 56,116,177,223,240,223,177,116, 56, 11,-15,-18, -7},
	{  0,-18,-38,-39,  0, 83,191,284,320,284,191, 83,  0,-39,-38,-18,  0},
	{  6, 20,  7,-42,-74,-12,159,353,440,353,159,-12,-74,-42,  7, 20,  6}
};

FaxDemodulator::FaxDemodulator(QObject* parent)
	: QObject(parent),
	  carrier(0), rate(0), deviation(0), fm(true), iFir(17), qFir(17)
{
	sine=new double[sine_size];
	for(int i=0; i<sine_size; i++) {
		sine[i]=sin(2.0*M_PI*i/sine_size);
	}
	
	iFir.setCoefficients(lpf[1]);
	qFir.setCoefficients(lpf[1]);

	asine=new double[asine_size];
	for(int i=0; i<asine_size; i++) {
		asine[i]=asin(2.0*i/asine_size-1.0)/2.0/M_PI;
	}
	init();
};

FaxDemodulator::~FaxDemodulator(void)
{
	delete[] asine;
	delete[] sine;
}

void FaxDemodulator::init(void)
{
	sin_phase=sine;
	cos_phase=sine+sine_size/4;
	ifirold=qfirold=0;
}

void FaxDemodulator::setCarrier(int carrier)
{
	this->carrier=carrier;
}

void FaxDemodulator::setSampleRate(int sampleRate)
{
	rate=sampleRate;
}

void FaxDemodulator::setDeviation(int dev)
{
	deviation=dev;
}

void FaxDemodulator::setFM(bool fm)
{
	this->fm=fm;
}

void FaxDemodulator::newSamples(short* audio, int n)
{
	int demod[n];
	for(int i=0; i<n; i++) {
		double ifirout=iFir.filterSample(audio[i]* *cos_phase);
		double qfirout=qFir.filterSample(audio[i]* *sin_phase);
		if(fm) {
			double abs=sqrt(qfirout*qfirout+ifirout*ifirout);
			ifirout/=abs;
			qfirout/=abs;
			if(abs>10000) {
				double y=qfirold*ifirout-ifirold*qfirout;
				y=(y+1.0)/2.0*asine_size;
				double x=static_cast<double>(rate)/deviation;
				x*=asine[static_cast<int>(y)];
				if(x<-1.0) {
					x=-1.0;
				} else if(x>1.0) {
					x=1.0;
				}
				demod[i]=static_cast<int>((x/2.0+0.5)*255.0);
			} else {
				demod[i]=0;
			}
		} else {
			ifirout/=96000;
			qfirout/=96000;
			demod[i]=static_cast<int>
				(sqrt(ifirout*ifirout+qfirout*qfirout));
		}

		ifirold=ifirout;
		qfirold=qfirout;

		if((sin_phase+=sine_size*carrier/rate) >= sine+sine_size) {
			sin_phase-=sine_size;
		}
		if((cos_phase+=sine_size*carrier/rate) >= sine+sine_size) {
			cos_phase-=sine_size;
		}
	}
	emit data(demod,n);
}

void FaxDemodulator::setFilter(int n)
{
	iFir.setCoefficients(lpf[static_cast<size_t>(n)]);
	qFir.setCoefficients(lpf[static_cast<size_t>(n)]);
}
