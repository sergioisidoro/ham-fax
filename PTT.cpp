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

#include "PTT.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "Error.hpp"

PTT::PTT(QObject* parent)
	: QObject(parent), device(0), usePTT(false)
{	
}

PTT::~PTT(void)
{
}

void PTT::setDeviceName(QString s)
{
	deviceName=s;
}

QString& PTT::getDeviceName(void)
{
	return deviceName;
}

void PTT::openDevice(void)
{
	if(usePTT) {
		if((device=open(deviceName,O_WRONLY))==-1) {
			throw Error(tr("could not open PTT device"));
		}
	}
}

void PTT::closeDevice(void)
{
	if(device!=0) {
		close(device);
		device=0;
	}
}

void PTT::set(bool press)
{
	if(usePTT) {
		int status;
		if(ioctl(device,TIOCMGET,&status)==-1) {
			throw Error(tr("error getting PTT state"));
		}
		if(press) {
			status|=TIOCM_RTS;
		} else {
			status&=~TIOCM_RTS;
		}
		if(ioctl(device,TIOCMSET,&status)==-1) {
			throw Error(tr("error setting PTT"));
		}
	}
}

void PTT::setUse(bool use)
{
	if(use!=usePTT) {
		usePTT=use;
		emit newPTTUsage(use );
	}
}
