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

#include "FaxModulator.hpp"
#include <math.h>

FaxModulator::FaxModulator(QObject* parent)
	: QObject(parent), sampleRate(0), carrier(0), dev(0)
{
	// put complete period of sine in lookup table
	sine=new signed short[size_sine];
	for(unsigned int i=0; i<size_sine; i++) {
		sine[i]=(signed short)
			(32767*sin(2.*M_PI*(double)i/(double)size_sine));
	}
}

FaxModulator::~FaxModulator(void)
{
	delete sine;
}

void FaxModulator::modulate(double* buffer, unsigned int number)
{
	signed short sample[number];
	for(unsigned int i=0; i<number; i++) {
		if(fm) {
			sample[i]=sine[phase];
			unsigned int f=static_cast<unsigned int>
				(carrier+2.0*(buffer[i]-0.5)*dev);
			phase+=size_sine*f/sampleRate;
		} else {
			sample[i]=static_cast<signed short>
				(0.04+0.96*buffer[i]*sine[phase]);
			phase+=size_sine*carrier/sampleRate;
		}
		phase%=size_sine;
	}
	emit data(sample,number);
}

void FaxModulator::setSampleRate(int sr)
{
	sampleRate=sr;
}

void FaxModulator::setCarrier(int carrier)
{
	this->carrier=carrier;
}

void FaxModulator::setDeviation(int dev)
{
	this->dev=dev;
}

void FaxModulator::setFM(bool fm)
{
	this->fm=fm;
}
