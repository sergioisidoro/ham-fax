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

#include "PTT.hpp"
#include <termios.h>
#include <sys/ioctl.h>

class Error {};

PTT::PTT(QObject* parent)
	: QObject(parent), usePTT(false)
{	
}

PTT::~PTT(void)
{
	device.close();
}

void PTT::setDeviceName(const QString& s)
{
	device.setName(s);
}

void PTT::set(void)
{
	if(usePTT) {
		try {
			if(!device.open(IO_WriteOnly)) {
				throw Error();
			}
			int status;
			if(ioctl(device.handle(),TIOCMGET,&status)==-1) {
				throw Error();
			}
			status|=TIOCM_RTS;
			if(ioctl(device.handle(),TIOCMSET,&status)==-1) {
				throw Error();
			}
		} catch(Error) {
			device.close();
		}
	}
}

void PTT::release(void)
{
	if(device.isOpen()) {
		try {
			int status;
			if(ioctl(device.handle(),TIOCMGET,&status)==-1) {
				throw Error();
			}
			status&=~TIOCM_RTS;
			if(ioctl(device.handle(),TIOCMSET,&status)==-1) {
				throw Error();
			}
			device.close();
		} catch(Error) {
			device.close();
		}
	}
}

void PTT::setUse(bool use)
{
	usePTT=use;
}
