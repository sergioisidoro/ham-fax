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

#include "FaxReceiver.hpp"
#include <math.h>

FaxReceiver::FaxReceiver(QObject* parent)
	: QObject(parent), state(APTSTART), sampleRate(0),
	  currentValue(0),
	  aptHigh(false), aptTrans(0), aptCount(0),
	  aptStartFreq(0), aptStopFreq(0), aptStop(false),
	  phaseHigh(false), currPhaseLength(0), color(false)
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
	phaseLines=noPhaseLines=0;
	lpm=lpmSum=0;
	emit searchingAptStart();
}

void FaxReceiver::decode(unsigned int* buf, unsigned int n)
{
	if(n==0) endReception();
	for(unsigned int i=0; i<n; i++) {
		currentValue=buf[i];
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
	if(x>230 && !aptHigh) {
		aptHigh=true;
		aptTrans++;
	} else if(x<22 && aptHigh) {
		aptHigh=false;
	}
	if(++aptCount >= sampleRate/2) {
		unsigned int f=sampleRate*aptTrans/aptCount;
		aptCount=aptTrans=0;
		emit aptFound(f);
		if(state==APTSTART) {
			if(f==aptStartFreq) {
				startPhasing();
			}
		} else {
			if(f==aptStopFreq) {
				if(aptStop) {
					endReception();
				} else {
					aptStop=true;
				}
			}
		}
	}
}

// Phasing lines consist of 2.5% white at the beginning, 95% black and again
// 2.5% white at the end (or inverted). In normal phasing lines we try to
// count the length between the white-black transitions. If the line has
// a reasonable amount of black (4.8%--5.2%) and the length fits in the 
// range of 60--360lpm (plus some tolerance) it is considered a valid
// phasing line. Then the start of a line and the lpm is calculated.

void FaxReceiver::decodePhasing(unsigned int& x)
{
	currPhaseLength++;
	if(x>128) {
		currPhaseHigh++;
	}
	if((!phaseInvers && x>230 && !phaseHigh) ||
	   ( phaseInvers && x<22  && phaseHigh)) {
		phaseHigh=phaseInvers?false:true;
	} else if((!phaseInvers && x<=128 && phaseHigh) ||
		  ( phaseInvers && x>=128 && !phaseHigh)) {
		phaseHigh=phaseInvers?true:false;
		if((double)currPhaseHigh >= 
		   (phaseInvers?0.948:0.048)*currPhaseLength &&
		   (double)currPhaseHigh <=
		   (phaseInvers?0.952:0.052)*currPhaseLength &&
		   (double)currPhaseLength/sampleRate<=1.1 &&
		   (double)currPhaseLength/sampleRate>=0.09) {
			double l=60.0*(double)sampleRate
				/(double)currPhaseLength;
			emit phasingLine(l);
			lpmSum+=l;
			++phaseLines;
			lpm=lpmSum/(double)phaseLines;
			imageSample=(int)(1.025*60.0/lpm*(double)sampleRate);
			
			noPhaseLines=0;
		} else if(phaseLines>0 && ++noPhaseLines>=5) {
			state=IMAGE;
			double pos=fmod(imageSample,sampleRate*60/lpm);
			pos/=(double)sampleRate*60.0/(double)lpm;
			lastCol=(unsigned int)(pos*width);
			pixel=pixelSamples=0;
			lastRow=99; // just !=0 which is the first row
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
		currRow=(unsigned int)((double)imageSample
				       /(double)sampleRate*lpm/60.0);
		if(col==lastCol) {
			pixel+=x;
			pixelSamples++;
		} else  {
			if(pixelSamples>0 && imageSample>0) {
				pixel/=pixelSamples;
				if(color) {
					emit newPixel(lastCol,
						      currRow/3,
						      pixel,
						      currRow%3);
				} else {
					emit newPixel(lastCol,currRow,
						      pixel,3);
				}
				if(lastRow!=currRow && state==IMAGE) {
					lastRow=currRow;
					emit imageRow(color ?
						      currRow/3 : currRow);
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
	phaseInvers=pol;
}

void FaxReceiver::startPhasing(void)
{
	if(state==APTSTART) {
		state=PHASING;
		phaseHigh = currentValue>=128 ? true : false;
		emit startingPhasing();
	}
}

// Here we want to remove the last detected phasing line and the following
// non phasing line from the beginning of the image and one second of apt stop
// from the end

void FaxReceiver::endReception(void)
{
	int h=currRow-(int)(lpm/60.0)-1;
	if(h>0) {
		emit newImageHeight(2,color ? h/3 : h);
	}
	state=DONE;
	emit receptionEnded();
}

void FaxReceiver::setColor(bool b)
{
	color=b;
}
