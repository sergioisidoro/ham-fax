// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt <cschmit@suse.de>
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

#include "FaxReceiver.hpp"
#include <math.h>

FaxReceiver::FaxReceiver(QObject* parent, FaxImage* image)
	: QObject(parent), state(APTSTART), image(image), sampleRate(0),
	  aptHigh(false), aptTrans(0), aptCount(0),
	  aptStartFreq(0), aptStopFreq(0), aptStop(false),
	  phaseHigh(false), currPhaseLength(0)
{
}

void FaxReceiver::setSampleRate(unsigned int rate)
{
	sampleRate=rate;
}

void FaxReceiver::init(void)
{
	state=APTSTART;
	aptCount=aptTrans=0;
	aptStop=false;
	currPhaseLength=currPhaseHigh=0;
	phaseLines=0;
	lpm=lpmSum=0;
	imageSample=0;
	emit statusText(tr("searching APT start tone"));
	pixelSamples=pixel=0;
}

void FaxReceiver::decode(unsigned int* buf, unsigned int n)
{
	for(unsigned int i=0; i<n; i++) {
		decodeApt(buf[i]);
		if(state==PHASING) {
			decodePhasing(buf[i]);
			decodeImage(buf[i]);
		} else if(state==IMAGE) {
			decodeImage(buf[i]);
		}
	}
}

void FaxReceiver::decodeApt(unsigned int& x)
{
	if(x>=128 && !aptHigh) {
		aptHigh=true;
		aptTrans++;
	} else if(x<128 && aptHigh) {
		aptHigh=false;
	}
	if(++aptCount >= sampleRate/2) {
		unsigned int f=sampleRate*aptTrans/aptCount;
		aptCount=aptTrans=0;
		emit aptText(QString(tr("Apt frequency: %1 Hz")).arg(f));
		if(state==APTSTART) {
			if(f>=aptStartFreq-1 && f<=aptStartFreq+1) {
				state=PHASING;
				emit statusText(tr("decoding phasing"));
				phaseHigh = x>=128 ? true : false;
			}
		} else {
			if(f>=aptStopFreq-2 && f<=aptStopFreq+2) {
				if(aptStop) {
					image->resize(0,0,image->getCols(),
						      lastRow-
						      (unsigned int)(lpm/60.0)+1);
					state=DONE;
					emit aptStopDetected();
				} else {
					aptStop=true;
				}
			}
		}
	}
}

void FaxReceiver::decodePhasing(unsigned int& x)
{
	currPhaseLength++;
	if(x>=128) {
		currPhaseHigh++;
	}
	if(( phasePol && x>=128 && !phaseHigh) ||
	   (!phasePol && x<=128 && phaseHigh)) {
		phaseHigh=phasePol?true:false;
	} else if(( phasePol && x<=128 && phaseHigh) ||
		  (!phasePol && x>=128 && !phaseHigh)) {
		phaseHigh=phasePol?false:true;
		if((double)currPhaseHigh >=
		   (phasePol?0.049:0.949)*currPhaseLength &&
		   (double)currPhaseHigh <=
		   (phasePol?0.051:0.951)*currPhaseLength &&
		   (double)currPhaseLength/sampleRate<=1.1 &&
		   (double)currPhaseLength/sampleRate>=0.09) {
			double l=60.0*(double)sampleRate
				/(double)currPhaseLength;
			emit statusText(QString(
				tr("phasing line, lpm %1")).arg(l,0,'f',1));
			lpmSum+=l;
			++phaseLines;
			lpm=lpmSum/(double)phaseLines;
			imageSample=(int)
				(-2.975*60.0/lpm*(double)sampleRate);
		} else if(phaseLines>0) {
			state=IMAGE;
			emit statusText(tr("receiving line 0"));
		}
		currPhaseLength=currPhaseHigh=0;
	}
}

void FaxReceiver::decodeImage(unsigned int& x)
{
	if(lpm>0) {
		double pos=fmod(imageSample,sampleRate*60/lpm);
		pos/=(double)sampleRate*60.0/(double)lpm;
		unsigned int col=(unsigned int)(pos*width);
		
		if(col==lastCol) {
			pixel+=x;
			pixelSamples++;
		} else  {
			if(pixelSamples>0 && imageSample>0) {
				unsigned int row=(unsigned int)
					((double)imageSample
					 /(double)sampleRate *lpm/60.0);
				pixel/=pixelSamples;
				if(!image->setPixelGray(lastCol,row,pixel)) {
					image->resize(0,0,image->getCols(),
						      image->getRows()+20);
					image->setPixelGray(lastCol,row,pixel);
				}
				if(lastRow!=row) {
					emit statusText(tr("receiving line %1")
							.arg(lastRow=row));
				}
			}
			lastCol=col;
			pixel=x;
			pixelSamples=1;
		}
	}
	imageSample++;
}

void FaxReceiver::setAptStartFreq(int f)
{
	aptStartFreq=f;
}

void FaxReceiver::setAptStopFreq(int f)
{
	aptStopFreq=f;
}

void FaxReceiver::setWidth(unsigned int width)
{
	this->width=width;
}

void FaxReceiver::setPhasePol(bool pol)
{
	this->phasePol=pol;
}

void FaxReceiver::skipAptStart(void)
{
	if(state==APTSTART) {
		state=PHASING;
	}
}
