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

#include "PTC.hpp"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <qtimer.h>
#include "Error.hpp"

PTC::PTC(QObject* parent) 
	: QObject(parent), deviceName("/dev/ttyS0"),
	  device(-1), fm(true), speed(38400), deviation(0), notifier(0)
{
}

PTC::~PTC(void)
{
	if(device!=-1) {
           	write(device,"\377",1); // return from fax mode
		::close(device);
	}
}

void PTC::setDeviceName(const QString& s)
{
	deviceName=s;
}

void PTC::open(void)
{
	device=::open(deviceName,O_RDWR|O_NOCTTY|O_NONBLOCK);
	
	struct termios options;
	tcgetattr(device,&options);
	switch(speed) {
	case 38400:
		cfsetispeed(&options,B38400);
		cfsetospeed(&options,B38400);
		break;
	case 57600:
		cfsetispeed(&options,B57600);
		cfsetospeed(&options,B57600);
		break;
	case 115200:
		cfsetispeed(&options,B115200);
		cfsetospeed(&options,B115200);
		break;
	}
	options.c_cflag|=CRTSCTS;
	options.c_cc[VMIN]=0;
	options.c_cc[VTIME]=0;
	cfmakeraw(&options);
	tcsetattr(device,TCSAFLUSH,&options);

	write(device,"\r\r",2);
	QString s=QString("FAX MBAUD %1\r").arg(speed);
	write(device,s,s.length());
	s=QString("FAX DEVIATION %1\r").arg(deviation);
	write(device,s,s.length());
	s=QString("FAX %1\r").arg(fm ? "JVCOMM :HamFax FM" : "AMFAX");
	write(device,s,s.length());
	usleep(300000);
	tcflush(device,TCIOFLUSH);
}

void PTC::startInput(void)
{
	open();
	notifier=new QSocketNotifier(device,QSocketNotifier::Read);
	connect(notifier,SIGNAL(activated(int)),this,SLOT(read(int)));
	emit newSampleRate(speed/10);
}

void PTC::startOutput(void)
{
	open();
	notifier=new QSocketNotifier(device,QSocketNotifier::Write);
	connect(notifier,SIGNAL(activated(int)),this,SLOT(checkSpace(int)));
	emit newSampleRate(speed/20);
}

void PTC::end(void)
{
	if(device!=-1) {
		notifier->setEnabled(false);
		if(notifier->type()==QSocketNotifier::Read) {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(read(int)));
			close();
		} else {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(checkSpace(int)));
			int i;
			ioctl(device,TIOCOUTQ,&i);
			QTimer::singleShot(1000*(i+8000)*20/speed,
					   this,SLOT(close()));
		}
		delete notifier;
	}
}

void PTC::close(void)
{
	write(device,"\377",1); // return from fax mode
	::close(device);
	device=-1;
	emit deviceClosed();
}

void PTC::transmit(double* samples, int count)
{
	try {
		notifier->setEnabled(false);
		unsigned char buf[count];
		for(int i=0; i<count; i++) {
			buf[i]=static_cast<unsigned char>
				(samples[i]*(fm?240.0:63.0));
		}
		tcflush(device,TCIFLUSH);
		if(write(device,buf,count)!=count) {
			throw Error();
		}
		notifier->setEnabled(true);
	} catch(Error) {
		close();
	}
}

void PTC::setDeviation(int dev)
{
	deviation=dev;
}

void PTC::setFM(bool fm)
{
	this->fm=fm;
}

void PTC::setSpeed(int s)
{
	speed=s;
}

void PTC::read(int fd)
{
	int n=512;
	unsigned char charbuffer[n];
	n=::read(device,charbuffer,n);
	int intbuffer[n];
	for(int i=0; i<n; i++) {
		intbuffer[i]=charbuffer[i];
	}
	emit data(intbuffer,n);
}

void PTC::checkSpace(int fd)
{
	emit spaceLeft(512);
}
