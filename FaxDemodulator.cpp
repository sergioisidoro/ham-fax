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

#include "FaxDemodulator.hpp"
#include <math.h>

FaxDemodulator::FaxDemodulator(QObject* parent)
	: QObject(parent), 
	carrier(0), rate(0), deviation(0), fm(true), ifirold(0), qfirold(0)
{
	sine=new double[sine_size];
	for(unsigned int i=0; i<sine_size; i++) {
		sine[i]=sin(2.*M_PI*(double)i/(double)sine_size);
	}
	sin_phase=sine;
	cos_phase=sine+sine_size/4;

	icurrent=ifir=new double[fir_size](0);
	ifir_end=ifir+fir_size;
	qcurrent=qfir=new double[fir_size](0);
	qfir_end=qfir+fir_size;

	asine=new double[asine_size];
	for(unsigned int i=0; i<asine_size; i++) {
		asine[i]=asin((double)i/(double)asine_size*2.0-1.0)/2.0/M_PI;
	}
}

FaxDemodulator::~FaxDemodulator(void)
{
	delete[] ifir;
	delete[] qfir;
	delete[] sine;
}

void FaxDemodulator::setCarrier(int carrier)
{
	this->carrier=carrier;
}

void FaxDemodulator::setSampleRate(int sampleRate)
{
	rate=(unsigned int)sampleRate;
}

void FaxDemodulator::setDeviation(int dev)
{
	deviation=(unsigned int)dev;
}

void FaxDemodulator::setFM(bool fm)
{
	this->fm=fm;
}

void FaxDemodulator::demodulate(unsigned int* demod,
				short* audio, unsigned int n)
{
	for(unsigned int i=0; i<n; i++) {
		*icurrent=(double)audio[i]* *cos_phase;
		*qcurrent=(double)audio[i]* *sin_phase;

		double ifirout=0;
		double qfirout=0;

		double* pi=icurrent;
		double* pq=qcurrent;
		for(unsigned int k=0; k<fir_size; k++) {
			ifirout+= *pi * lpf[k];
			qfirout+= *pq * lpf[k];
			if(++pi>=ifir_end) {
				pi=ifir;
			}
			if(++pq>=qfir_end) {
				pq=qfir;
			}
		}

		if(fm) {
			double abs=sqrt(qfirout*qfirout+ifirout*ifirout);
			ifirout/=abs;
			qfirout/=abs;
			
			double y=qfirold*ifirout-ifirold*qfirout;
			double x=(double)rate/(double)deviation
				*asine[(unsigned int)
				      ((y+1.0)/2.0*asine_size)];
			if(x<-1.0) {
				x=-1.0;
			} else if(x>1.0) {
				x=1.0;
			}
			demod[i]=(unsigned char)((x/2.0+0.5)*255.0);
		} else {
			ifirout/=96000;
			qfirout/=96000;
			demod[i]=(unsigned char)
				sqrt(ifirout*ifirout+qfirout*qfirout);
		}

		ifirold=ifirout;
		qfirold=qfirout;
		if(++icurrent>=ifir_end) {
			icurrent=ifir;
		}
		if(++qcurrent>=qfir_end) {
			qcurrent=qfir;
		}
		if((sin_phase+=sine_size*carrier/rate) >= sine+sine_size) {
			sin_phase-=sine_size;
		}
		if((cos_phase+=sine_size*carrier/rate) >= sine+sine_size) {
			cos_phase-=sine_size;
		}
	}
}
