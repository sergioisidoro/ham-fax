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

#include "Sound.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include <qtimer.h>
#include "Config.hpp"
#include "Error.hpp"

Sound::Sound(QObject* parent)
	: QObject(parent), sampleRate(8000), dsp(-1), notifier(0)
{
}

Sound::~Sound(void)
{
	if(dsp!=-1) {
		::close(dsp);
	}
}

int Sound::startOutput(void)
{
	try {
		QString devDSPName=Config::instance().
			readEntry("/hamfax/sound/device");
		if((dsp=open(devDSPName,O_WRONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_NE;
		if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_NE) {
			throw Error(
				tr("could not set audio format to S16_NE"));
		}
		int channels=1;
		if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		int speed=sampleRate;
		ioctl(dsp,SNDCTL_DSP_SPEED,&speed);
		if(speed<sampleRate*0.99 || speed>sampleRate*1.01) {
			throw Error(tr("could not set sample rate"));
		}
		notifier=new QSocketNotifier(dsp,QSocketNotifier::Write,this);
		connect(notifier,SIGNAL(activated(int)),
			this,SLOT(checkSpace(int)));
		ptt.set();
	} catch(Error) {
		close();
		throw;
	}
	return sampleRate;
}

int Sound::startInput(void)
{
	try {
		QString devDSPName=Config::instance().
			readEntry("/hamfax/sound/device");
		if((dsp=open(devDSPName,O_RDONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_LE;
		if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_LE) {
			throw Error(
				tr("could not set audio format S16_LE"));
		}
		int channels=1;
		if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		int speed=sampleRate;
		ioctl(dsp,SNDCTL_DSP_SPEED,&speed);
		if(speed<sampleRate*0.99 || speed>sampleRate*1.01) {
			throw Error(tr("could not set sample rate"));
		}
		notifier=new QSocketNotifier(dsp,QSocketNotifier::Read,this);
		connect(notifier,SIGNAL(activated(int)),SLOT(read(int)));
	} catch(Error) {
		close();
		throw;
	}
	return sampleRate;
}

void Sound::end(void)
{
	if(dsp!=-1) {
		notifier->setEnabled(false);
		if(notifier->type()==QSocketNotifier::Read) {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(read(int)));
			close();
		} else {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(checkSpace(int)));
			int i;
			ioctl(dsp,SNDCTL_DSP_GETODELAY,&i);
			QTimer::singleShot(1000*i
					   /sampleRate/sizeof(short),
					   this,SLOT(close()));
		}
		delete notifier;
	}
}

void Sound::write(short* samples, int number)
{
	try {
		if(dsp!=-1) {
			notifier->setEnabled(false);
			if((::write(dsp,samples, number*sizeof(short)))
			   !=static_cast<int>(number*sizeof(short))) {
				throw Error();
			}
			notifier->setEnabled(true);
		}
	} catch(Error) {
		close();
	}
}

void Sound::read(int fd)
{
	const int max=256;
	short buffer[max];
	int n=::read(fd,buffer,max*sizeof(short))/sizeof(short);
	if(n>0 && n<=max) {
		emit data(buffer,n);
	}
}

void Sound::checkSpace(int fd)
{
	audio_buf_info info;
	ioctl(fd,SNDCTL_DSP_GETOSPACE,&info);
	emit spaceLeft(info.bytes/sizeof(short));
}

void Sound::close(void)
{
	ioctl(dsp,SNDCTL_DSP_RESET);
	::close(dsp);
	dsp=-1;
	emit deviceClosed();
	ptt.release();
}

void Sound::closeNow(void)
{
	if(dsp!=-1) {
		notifier->setEnabled(false);
		delete notifier;
		close();
	}
}
