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

#include "FaxTransmitter.hpp"
#include <math.h>

FaxTransmitter::FaxTransmitter(QObject* parent, FaxImage* faxImage)
	: QObject(parent), 
	faxImage(faxImage),
	state(IDLE),
	sampleNr(0),
	lpm(0),
	startLength(0),
	startFreq(0),
	phasingLines(0),
	stopLength(0), 
	stopFreq(0)
{
}

void FaxTransmitter::startTransmission(void)
{
	state=APTSTART;
	sampleNr=0;
	emit aptStart();
}

void FaxTransmitter::getValues(double* buf, unsigned int& maxSamples) 
{
	unsigned int i;
	for(i=0; i<maxSamples; i++) {
		if(state==IDLE) {
			i=0;
			maxSamples=i;
			break;
		}
		if(state==APTSTART) {
			if(sampleNr>=sampleRate*startLength) {
				state=PHASING;
				sampleNr=0;
				emit phasing();
			} else {
				// black/white pattern with startFreq
				buf[i]=(sampleNr*2*startFreq/sampleRate)%2;
				sampleNr++;
			}
		}
		if(state==PHASING) {
			if(sampleNr>=sampleRate*phasingLines*60/lpm) {
				state=ENDPHASING;
				sampleNr=0;
			} else {
				// determine current position in line
                                // (0.0 ... 1.0) and build line with
				// 2.5% white, 95% black and 2.5% white
				double pos=fmod(sampleNr,
						sampleRate*60/lpm);
				pos/=(double)sampleRate*60.0/(double)lpm;
				buf[i] = pos<0.025||pos>=0.975 
					? phasePol?1.0:0.0 
					: phasePol?0.0:1.0;
				sampleNr++;
			}
		}
		if(state==ENDPHASING) {
			if(sampleNr>=sampleRate*60/lpm) {
				state=IMAGE;
				sampleNr=0;
			} else {
				double pos=fmod(sampleNr,
						sampleRate*60/lpm);
				pos/=(double)sampleRate*60.0/(double)lpm;
				buf[i]= phasePol?1.0:0.0;
				sampleNr++;
			}
		}
		if(state==IMAGE) {
			if(sampleNr>=sampleRate*faxImage->getRows()*60/lpm) {
				state=APTSTOP;
				sampleNr=0;
				emit aptStop();
			} else {
				// get pixel determining current value
				double pos=fmod(sampleNr,sampleRate*60/lpm);
				pos/=(double)sampleRate*60.0/(double)lpm;
				unsigned int c=(unsigned int)
					(pos*faxImage->getCols());
				unsigned int r=sampleNr*lpm
					/60/sampleRate;
				buf[i]=(double)
					faxImage->getPixelGray(c,r)/255.0;
				sampleNr++;
			}
		}
		if(state==APTSTOP) {
			if(sampleNr>=sampleRate*stopLength) {
				state=IDLE;
				maxSamples=i;
				break;
			} else {
				// black/white pattern with stopFreq
				buf[i]=sampleNr*2*stopFreq/sampleRate%2;
				sampleNr++;
			}
		}
	}
	if(state==IMAGE) {
		emit imageLine(sampleNr*lpm/60/sampleRate);
	}
}

void FaxTransmitter::setLPM(unsigned int lpm)
{
	this->lpm=lpm;
}

void FaxTransmitter::setAptStartFreq(unsigned int f)
{
	startFreq=f;
}

void FaxTransmitter::setAptStartLength(unsigned int t)
{
	startLength=t;
}

void FaxTransmitter::setAptStopFreq(unsigned int f)
{
	stopFreq=f;
}

void FaxTransmitter::setAptStopLength(unsigned int t)
{
	stopLength=t;
}

void FaxTransmitter::setPhasingLines(unsigned int n)
{
	phasingLines=n;
}

void FaxTransmitter::setPhasePol(bool pol)
{
	phasePol=pol;
}

void FaxTransmitter::setSampleRate(unsigned int rate)
{
	sampleRate=rate;
}
