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

#include "FaxReceiver.hpp"
#include <math.h>

FaxReceiver::FaxReceiver(QObject* parent)
	: QObject(parent), sampleRate(0), aptStartFreq(0), aptStopFreq(0), 
	color(false), rawData(0)
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
	aptStop=aptHigh=false;
	imageSample=0;
	rawData.resize(1024*1024*8);
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
		}
		if((state==PHASING||state==IMAGE)&&lpm>0) {
			if(static_cast<int>(rawData.size())<=imageSample) {
				rawData.resize(rawData.size()+1024*1024);
			}
			decodeImage(buf[i]);
		}
	}
}

// The number of transistions between black and white is counted. After 1/2 
// second, the frequency is calculated. If it matches the APT start frequency,
// the state skips to the detection of phasing lines, if it matches the apt
// stop frequency two times, the reception is ended.

void FaxReceiver::decodeApt(const int& x)
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

void FaxReceiver::decodePhasing(const int& x)
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
		   static_cast<double>(currPhaseLength)<=1.1*sampleRate &&
		   static_cast<double>(currPhaseLength)>=0.15*sampleRate) {
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
		} else if(currPhaseLength>5*sampleRate) {
			currPhaseLength=0;
		}
		currPhaseLength=currPhaseHigh=0;
	}
}

void FaxReceiver::decodeImage(const int& x)
{
	int col=static_cast<int>(width*fmod(imageSample,sampleRate*60/lpm)
				 /sampleRate/60.0*lpm);
	int currRow=static_cast<int>(imageSample*lpm/60.0/sampleRate);
	rawData[imageSample]=x;
	if(col==lastCol) {
		pixel+=x;
		pixelSamples++;
	} else {
		if(pixelSamples>0) {
			pixel/=pixelSamples;
			emit setPixel(lastCol, color?currRow/3:currRow, 
				      pixel,color?currRow%3:3);
			if(lastRow!=currRow && state!=PHASING) {
				emit row((lastRow=currRow)/(color?3:1));
			}
		}
		lastCol=col;
		pixel=x;
		pixelSamples=1;
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
			endReception();
			break;
		}
		decodeImage(*rawIt);
	}
}

void FaxReceiver::skip(void)
{
	if(state==APTSTART) {
		state=PHASING;
		phaseHigh = currentValue>=128 ? true : false;
		currPhaseLength=currPhaseHigh=0;
		phaseLines=noPhaseLines=0;
		lpm=lpmSum=0;
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
	int h=lastRow-static_cast<int>(lpm/60.0)-1;
	rawData.resize(imageSample);
	if(h>0) {
		emit newSize(0,2,0,color ? h/3 : h);
		emit bufferNotEmpty(true);
	}
	state=DONE;
	emit end();
}

void FaxReceiver::releaseBuffer(void)
{
	rawData.resize(0);
	emit bufferNotEmpty(false);
}

void FaxReceiver::setColor(bool b)
{
	color=b;
}

void FaxReceiver::setTxLPM(int lpm)
{
	txLPM=lpm;
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
