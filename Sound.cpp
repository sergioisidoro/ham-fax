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

#include "Sound.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include "Error.hpp"

Sound::Sound(QObject* parent)
	: QObject(parent), devDSP(-1), devDSPName("/dev/dsp")
{
}

Sound::~Sound(void)
{
	this->close();
}

void Sound::setDSPDevice(QString s)
{
	devDSPName=s;
}

QString& Sound::getDSPDevice(void)
{
	return devDSPName;
}

void Sound::openOutput(unsigned int sampleRate)
{
	try {
		if((devDSP=open(devDSPName,O_WRONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_LE;
		if(ioctl(devDSP,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_LE) {
			throw Error(
				tr("could not set audio format to S16_LE"));
		}
		int channels=1;
		if(ioctl(devDSP,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		int speed=(int)sampleRate;
		if(ioctl(devDSP,SNDCTL_DSP_SPEED,&speed)==-1
		   ||speed!=(int)sampleRate) {
			throw Error(tr("could not set sample rate"));
		}
		int flags=fcntl(devDSP,F_GETFL);
		if(fcntl(devDSP,F_SETFL,flags&~O_NONBLOCK)==-1) {
			throw Error(tr("could not set blocking mode"));
		}
	} catch(Error) {
		close();
		throw;
	}
}

void Sound::openInput(unsigned int sampleRate)
{
	try {
		if((devDSP=open(devDSPName,O_RDONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_LE;
		if(ioctl(devDSP,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_LE) {
			throw Error(
				tr("could not set audio format S16_LE"));
		}
		int channels=1;
		if(ioctl(devDSP,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		int speed=sampleRate;
		if(ioctl(devDSP,SNDCTL_DSP_SPEED,&speed)==-1
		   ||speed!=(int)sampleRate) {
			throw Error(tr("could not set sample rate"));
		}
		int flags=fcntl(devDSP,F_GETFL);
		if(fcntl(devDSP,F_SETFL,flags&~O_NONBLOCK)==-1) {
			throw Error(tr("could not set blocking mode"));
		}
	} catch(Error) {
		close();
		throw;
	}
}

void Sound::close(void)
{
	if(devDSP!=-1) {
		ioctl(devDSP,SNDCTL_DSP_RESET);
		::close(devDSP);
		devDSP=-1;
	}
}

void Sound::write(signed short* samples, unsigned int number)
{
	try {
		if(devDSP!=-1) {
			if((::write(devDSP,samples,
				    number*sizeof(signed short)))
			   !=(int)(number*sizeof(signed short))) {
				throw Error(tr("could not write to DSP"));
			}
		}
	} catch(Error) {
		close();
		throw;
	}
}

void Sound::read(signed short* samples, unsigned int& number)
{
	try {
		if(devDSP!=-1) {
			if(::read(devDSP,samples,
				  number*sizeof(signed short))==-1) {
				throw Error(tr("could not read from DSP"));
			}
		}
	} catch(Error) {
		close();
		throw;
	}
}

bool Sound::outputBufferEmpty(void)
{
	if(devDSP!=-1) {
		int i;
		ioctl(devDSP,SNDCTL_DSP_GETODELAY,&i);
		return i>0 ? false : true;
	}
	return true;
}
