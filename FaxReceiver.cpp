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
	  phaseHigh(false), currPhaseLength(0), color(false),
	  rawData(0)
{
	timer=new QTimer(this);
	connect(timer,SIGNAL(timeout()),this,SLOT(adjustNext()));
}

void FaxReceiver::setSampleRate(int rate)
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
	rawData.resize(8388608);
	emit startReception();
}

void FaxReceiver::decode(int* buf, int n)
{
	if(n==0) endReception();
	for(int i=0; i<n; i++) {
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

// The number of transistions between black and white is counted. After 1/2 
// second, the frequency is calculated. If it matches the APT start frequency,
// the state skips to the detection of phasing lines, if it matches the apt
// stop frequency two times, the reception is ended.

void FaxReceiver::decodeApt(int& x)
{
	if(x>229 && !aptHigh) {
		aptHigh=true;
		aptTrans++;
	} else if(x<25 && aptHigh) {
		aptHigh=false;
	}
	if(++aptCount >= sampleRate/2) {
		int f=sampleRate*aptTrans/aptCount;
		aptCount=aptTrans=0;
		emit aptFound(f);
		if(state==APTSTART) {
			if(f==aptStartFreq) {
				skip();
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

void FaxReceiver::decodePhasing(int& x)
{
	currPhaseLength++;
	if(x>128) {
		currPhaseHigh++;
	}
	if((!phaseInvers && x>229 && !phaseHigh) ||
	   ( phaseInvers && x<25  && phaseHigh)) {
		phaseHigh=phaseInvers?false:true;
	} else if((!phaseInvers && x<25 && phaseHigh) ||
		  ( phaseInvers && x>229 && !phaseHigh)) {
		phaseHigh=phaseInvers?true:false;
		if(currPhaseHigh>=(phaseInvers?0.948:0.048)*currPhaseLength &&
		   currPhaseHigh<=(phaseInvers?0.952:0.052)*currPhaseLength &&
		   static_cast<double>(currPhaseLength)/sampleRate<=1.1 &&
		   static_cast<double>(currPhaseLength)/sampleRate>=0.09) {
			double l=60.0*sampleRate/currPhaseLength;
			emit phasingLine(l);
			lpmSum+=l;
			++phaseLines;
			lpm=lpmSum/phaseLines;
			imageSample=static_cast<int>(1.025*60./lpm*sampleRate);
			noPhaseLines=0;
		} else if(phaseLines>0 && ++noPhaseLines>=5) {
			state=IMAGE;
			double pos=fmod(imageSample,sampleRate*60/lpm);
			pos/=sampleRate*60.0/lpm;
			lastCol=static_cast<int>(pos*width);
			pixel=pixelSamples=0;
			lastRow=99; // just !=0 which is the first row
			emit imageStarts();
		}
		currPhaseLength=currPhaseHigh=0;
	}
}

void FaxReceiver::decodeImage(int& x)
{
	if(lpm>0) {
		double pos=fmod(imageSample,sampleRate*60/lpm);
		pos/=sampleRate*60.0/lpm;
		int col=static_cast<int>(pos*width);
		currRow=static_cast<int>
			(static_cast<double>(imageSample)/sampleRate*lpm/60.0);
		int rawSize=rawData.size();
		if(rawSize<=imageSample) {
			rawData.resize(rawSize+1024*1024);
		}
		rawData[imageSample]=x;
		if(col==lastCol) {
			pixel+=x;
			pixelSamples++;
		} else  {
			if(pixelSamples>0) {
				pixel/=pixelSamples;
				if(color) {
					emit newPixel(lastCol,currRow/3,
						      pixel,currRow%3);
				} else {
					emit newPixel(lastCol,currRow,pixel,3);
				}
				if(lastRow!=currRow && state==IMAGE) {
					lastRow=currRow;
					emit imageRow(color?currRow/3:currRow);
				}
			}
			lastCol=col;
			pixel=x;
			pixelSamples=1;
		}
	}
	imageSample++;
}

void FaxReceiver::correctLPM(double d)
{
	pixel=pixelSamples=imageSample=0;
	lastCol=99;
	lpm*= 1.0 + (color ? d/3.0 : d);
	rawIt=rawData.begin();
	timer->start(0);
	emit redrawStarts();
}

void FaxReceiver::correctWidth(int w)
{
	pixel=pixelSamples=imageSample=0;
	lastCol=99;
	width=w;
	if(rawData.isNull()) {
		emit imageWidth(w);
	} else {
		rawIt=rawData.begin();
		timer->start(0);
		emit newSize(0,0,w,0);
		emit redrawStarts();
	}
}

void FaxReceiver::adjustNext(void)
{
	for(int i=0; i<512; i++) {
		if(rawIt++>=rawData.end()) {
			timer->stop();
			int h=currRow-static_cast<int>(lpm/60.0)-1;
			emit newSize(0,2,0,color ? h/3 : h);
			emit end();
			break;
		}
		double pos=fmod(imageSample,sampleRate*60.0/lpm);
		pos/=sampleRate*60.0/lpm;
		int col=static_cast<int>(pos*width);
		currRow=static_cast<int>(imageSample*lpm/60.0/sampleRate);
		if(col==lastCol) {
			pixel+=*rawIt;
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
			}
			lastCol=col;
			pixel=*rawIt;
			pixelSamples=1;
		}
		imageSample++;
	}
}

void FaxReceiver::setAptStartFreq(int f)
{
	aptStartFreq=f;
}

void FaxReceiver::setAptStopFreq(int f)
{
	aptStopFreq=f;
}

void FaxReceiver::setWidth(int width)
{
	this->width=width;
}

void FaxReceiver::setPhasePol(bool pol)
{
	phaseInvers=pol;
}

void FaxReceiver::skip(void)
{
	if(state==APTSTART) {
		state=PHASING;
		phaseHigh = currentValue>=128 ? true : false;
		emit startingPhasing();
	} else if(state==PHASING) {
		lpm=txLPM;
		state=IMAGE;
		emit imageStarts();
		double pos=fmod(imageSample,sampleRate*60/lpm);
		pos/=sampleRate*60.0/lpm;
		lastCol=static_cast<int>(pos*width);
		pixel=pixelSamples=imageSample=0;
		lastRow=99; // just !=0 which is the first row
	}
}

// Here we want to remove the last detected phasing line and the following
// non phasing line from the beginning of the image and one second of apt stop
// from the end

void FaxReceiver::endReception(void)
{
	int h=currRow-static_cast<int>(lpm/60.0)-1;
	rawData.resize(imageSample);
	if(h>0) {
		emit newSize(0,2,0,color ? h/3 : h);
		emit bufferNotEmpty(true);
	}
	state=DONE;
	emit end();
}

void FaxReceiver::setColor(bool b)
{
	color=b;
}

void FaxReceiver::releaseBuffer(void)
{
	rawData.resize(0);
	emit bufferNotEmpty(false);
}

void FaxReceiver::setTxLPM(int lpm)
{
	txLPM=lpm;
}
