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

#include "Config.hpp"
#include "FaxTransmitter.hpp"
#include <cmath>

FaxTransmitter::FaxTransmitter(QObject* parent, FaxImage* faxImage)
	: QObject(parent), image(faxImage)
{
}

void FaxTransmitter::start(int sampleRate)
{
	Config& config=Config::instance();
	startLength=config.readNumEntry("/hamfax/APT/startLength");
	startFreq=config.readNumEntry("/hamfax/APT/startFrequency");
	stopLength=config.readNumEntry("/hamfax/APT/stopLength");
	stopFreq=config.readNumEntry("/hamfax/APT/stopFrequency");
	lpm=config.readNumEntry("/hamfax/fax/LPM");
	phasingLines=config.readNumEntry("/hamfax/phasing/lines");
	phaseInvers=config.readBoolEntry("/hamfax/phasing/invert");
	color=config.readBoolEntry("/hamfax/fax/color");
	this->sampleRate=sampleRate;
	state=APTSTART;
	sampleNr=0;
}

void FaxTransmitter::doNext(int n)
{
	// maximum number of samples to avoid stack overrun
	n = std::min(n, 1024);

	double lineSamples=60.0*sampleRate/lpm;
	double buf[n];
	for(int i=0; i<n; i++) {
		if(state==APTSTART) {
			if(sampleNr<sampleRate*startLength) {
				buf[i]=(sampleNr*2*startFreq/sampleRate)%2;
				sampleNr++;
			} else {
				sampleNr=0;
				emit phasing();
				state=PHASING;
			}
		}
		if(state==PHASING) {
			if(sampleNr<lineSamples*phasingLines) {
				double pos=std::fmod(sampleNr,lineSamples)
					/lineSamples;
				buf[i] = (pos<0.025||pos>=0.975 )
					? (phaseInvers?0.0:1.0) 
					: (phaseInvers?1.0:0.0);
				sampleNr++;
			} else {
				state=ENDPHASING;
				sampleNr=0;
			}
		}
		if(state==ENDPHASING) {
			if(sampleNr<lineSamples) {
				buf[i]= phaseInvers?0.0:1.0;
				sampleNr++;
			} else {
				state=IMAGE;
				sampleNr=0;
				row=0;
			}
		}
		if(state==IMAGE) {
			if(sampleNr<(color?3:1)*lineSamples*rows) {
				int c;
				c=static_cast<int>(std::fmod(sampleNr,lineSamples)
						   /lineSamples*cols);
				int r=static_cast<int>(sampleNr/lineSamples);
				if(row!=r) {
					emit imageLine((row=r)/(color?3:1));
				}
				buf[i]=image->getPixel(c, color? r/3:r,
						       color? r%3:3)/255.0;
				sampleNr++;
			} else {
				state=APTSTOP;
				sampleNr=0;
				emit aptStop();
			}
		}
		if(state==APTSTOP) {
			if(sampleNr<sampleRate*stopLength) {
				buf[i]=sampleNr*2*stopFreq/sampleRate%2;
				sampleNr++;
			} else {
				state=IDLE;
				n=i;
				emit end();
				break;
			}
		}
		if(state==IDLE) {
			n=0;
			break;
		}
	}
	emit data(buf,n);
}

void FaxTransmitter::setImageSize(int cols, int rows)
{
	this->rows=rows;
	this->cols=cols;
}

void FaxTransmitter::doAptStop(void)
{
	state=APTSTOP;
	sampleNr=0;
	emit aptStop();
}
