// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmittt, DH1CS <cschmit@suse.de>
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
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "Error.hpp"

PTC::PTC(QObject* parent) 
	: QObject(parent), device(-1), fm(true)
{
}

PTC::~PTC(void)
{
	close();
}

void PTC::setDeviceName(QString s)
{
	deviceName=s;
}

QString& PTC::getDeviceName(void)
{
	return deviceName;
}

void PTC::setFM(bool fm)
{
	this->fm=fm;
}

void PTC::open(void)
{
	device=::open(deviceName,O_RDWR|O_NOCTTY|O_NONBLOCK);
	int flags=fcntl(device,F_GETFL);
	if(fcntl(device,F_SETFL,flags&~O_NONBLOCK)==-1) {
		throw Error(tr("could not set blocking mode"));
	}
	
	struct termios options;
	tcgetattr(device,&options);
	cfsetispeed(&options,B57600);
	cfsetospeed(&options,B57600);
	options.c_cc[VMIN]=0;
	options.c_cc[VTIME]=1;
	cfmakeraw(&options);
	tcsetattr(device,TCSAFLUSH,&options);

	write(device,"\r\r",2);
	const char* s="FAX MBAUD 57600\r";
	write(device,s,strlen(s));
	if(fm) {
		const char* s="FAX FMFAX\r";
		write(device,s,strlen(s));
	} else {
		const char* s="FAX AMFAX\r";
		write(device,s,strlen(s));
	}
	usleep(200000);
	tcflush(device,TCIOFLUSH);
}

void PTC::close(void)
{
	if(device!=-1) {
		write(device,"\377",1); // return from fax mode
		::close(device);
		device=-1;
	}
}

void PTC::receive(unsigned int* samples, unsigned int& count)
{
	unsigned char buf[count];
	count=read(device,buf,count);
	for(unsigned int i=0; i<count; i++) {
		samples[i]=buf[i];
	}
}

void PTC::transmit(double* samples, unsigned int count)
{
	char buf[count];
	for(unsigned int i=0; i<count; i++) {
		buf[i]=(char)(samples[i]*63.0);
	}
	tcflush(device,TCIFLUSH);
	write(device,buf,count);
}
