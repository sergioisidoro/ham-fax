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

#include "FaxModulator.hpp"
#include <math.h>

FaxModulator::FaxModulator(QObject* parent)
	: QObject(parent), sampleRate(0), carrier(0), dev(0), sine(8192)
{
	for(size_t i=0; i<sine.size(); i++) {
		sine[i]=static_cast<short>(32767*sin(2.0*M_PI*i/sine.size()));
	}
}

FaxModulator::~FaxModulator(void)
{
}

void FaxModulator::init(void)
{
	sine.reset();
	if(!fm) {
		sine.setIncrement(sine.size()*carrier/sampleRate);
	}
}

void FaxModulator::modulate(double* buffer, int number)
{
	short sample[number];
	for(size_t i=0; i<static_cast<size_t>(number); i++) {
		if(fm) {
			int f=static_cast<int>(carrier+2.*(buffer[i]-0.5)*dev);
			sine.setIncrement(sine.size()*f/sampleRate);
			sample[i]=sine.nextValue();
		} else {
			sample[i]=static_cast<short>(sine.nextValue()*buffer[i]);
		}
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
