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
static const double lowPassFilter[3][17]={
	{ -7,-18,-15, 11, 56,116,177,223,240,223,177,116, 56, 11,-15,-18, -7},
	{  0,-18,-38,-39,  0, 83,191,284,320,284,191, 83,  0,-39,-38,-18,  0},
	{  6, 20,  7,-42,-74,-12,159,353,440,353,159,-12,-74,-42,  7, 20,  6}
};


FaxDemodulator::FaxDemodulator(QObject* parent)
	: QObject(parent),
	  carrier(0), rate(1), deviation(0), fm(true), 
	  iFir(17), qFir(17), sine(8192), cosine(8192), arcSine(256)
{
	iFir.setCoefficients(lowPassFilter[1]);
	qFir.setCoefficients(lowPassFilter[1]);
	for(size_t i=0; i<sine.size(); i++) {
		sine[i]=sin(2.0*M_PI*i/sine.size());
	}
	for(size_t i=0; i<cosine.size(); i++) {
		cosine[i]=cos(2.0*M_PI*i/cosine.size());
	}
	for(size_t i=0; i<arcSine.size(); i++) {
		arcSine[i]=asin(2.0*i/arcSine.size()-1.0)/2.0/M_PI;
	}
	init();
};

FaxDemodulator::~FaxDemodulator(void)
{
}

void FaxDemodulator::init(void)
{
	sine.reset();
	cosine.reset();
	ifirold=qfirold=0;
}

void FaxDemodulator::setCarrier(int carrier)
{
	this->carrier=carrier;
	sine.setIncrement(sine.size()*carrier/rate);
	cosine.setIncrement(cosine.size()*carrier/rate);
}

void FaxDemodulator::setSampleRate(int sampleRate)
{
	rate=sampleRate;
	sine.setIncrement(sine.size()*carrier/rate);
	cosine.setIncrement(cosine.size()*carrier/rate);
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
		double ifirout=iFir.filterSample(audio[i]*cosine.nextValue());
		double qfirout=qFir.filterSample(audio[i]*sine.nextValue());
		if(fm) {
			double abs=sqrt(qfirout*qfirout+ifirout*ifirout);
			ifirout/=abs;
			qfirout/=abs;
			if(abs>10000) {
				double y=qfirold*ifirout-ifirold*qfirout;
				y=(y+1.0)/2.0*arcSine.size();
				double x=static_cast<double>(rate)/deviation;
				x*=arcSine[static_cast<size_t>(y)];
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
	}
	emit data(demod,n);
}

void FaxDemodulator::setFilter(int n)
{
	iFir.setCoefficients(lowPassFilter[static_cast<size_t>(n)]);
	qFir.setCoefficients(lowPassFilter[static_cast<size_t>(n)]);
}
