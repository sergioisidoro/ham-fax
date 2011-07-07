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

#include "PTT.hpp"
#include <termios.h>
#include <sys/ioctl.h>
#include "Config.hpp"
#include "Error.hpp"

#ifdef HAVE_LIBHAMLIB
#include <stdarg.h>
#include <vector>
#include <iostream>
#include <qmessagebox.h>
#endif

#ifdef HAVE_LIBHAMLIB
static void ThrowError( const char * formatString, ... )
{
	va_list args;
	va_start( args, formatString );
	char tmpBuffer[2048];
	vsprintf( tmpBuffer, formatString, args );
	va_end( args );
	std::cerr << "Throw:" << tmpBuffer << "\n" ;
	/// tr() should be applied to the string but I prefer not to have PTT inherit from QObject.
	throw Error( tmpBuffer );
};

bool PTT::hamlibInit(void)
{
	if (hamlibRigPtr){
		return true;
	}
std::cout << this << " ctor " << __FUNCTION__ << "\n" ;
	Config& c=Config::instance();
	std::string hamlibModelString = c.readEntry("/hamfax/HAMLIB/hamlib_model");
	if( hamlibModelString.empty() )
	{
		return false ;
	};

	bool hamlibModelOk ;
	int hamlibModel = c.readNumEntry("/hamfax/HAMLIB/hamlib_model", 0, &hamlibModelOk );

	if (!hamlibModelOk) {
		ThrowError("Hamlib: Model is not an integer:%s", hamlibModelString.c_str() );
	};

	hamlibRigPtr = rig_init(hamlibModel);
	if( ! hamlibRigPtr ) {
		ThrowError("Hamlib: rig_init invalid model=%d", hamlibModel);
	}

	std::string pttDeviceString = c.readEntry("/hamfax/PTT/device").ascii();
	if( pttDeviceString.size() >= FILPATHLEN ) {
		ThrowError("Hamlib: rig_init PTT device too long=%s", pttDeviceString.c_str() );
	};
std::cout << "pttDev=" << pttDeviceString << "\n" ;
	strncpy(hamlibRigPtr->state.rigport.pathname, pttDeviceString.c_str(), FILPATHLEN);

	QString hamlibParametersQString = c.readEntry("/hamfax/HAMLIB/hamlib_parameters");
	std::vector<char> hamlibParametersVector(
			hamlibParametersQString.ascii(),
			hamlibParametersQString.ascii() +
			hamlibParametersQString.length() );

	/// It must be terminated with a zero.
	char *ptrParams = &hamlibParametersVector[0] ;
std::cout << "pttDev=" << ptrParams << "\n" ;

	/// Parses the optional hamlib arguments separated by commas.
	while (ptrParams && *ptrParams != '\0') {
		char * ptrVal = strchr(ptrParams, '=');
		if (ptrVal) {
			*ptrVal++ = '\0';
		}

		char * ptrParamsNext = ptrVal ? strchr(ptrVal, ',') : NULL ;
		if (ptrParamsNext) {
			*ptrParamsNext++ = '\0';
		}
		int hamlibRigError = rig_set_conf(hamlibRigPtr,
				    rig_token_lookup(hamlibRigPtr, ptrParams), ptrVal);
		if (hamlibRigError != RIG_OK) {
			rig_cleanup(hamlibRigPtr);
			hamlibRigPtr = NULL ;
			ThrowError("Hamlib: rig_set_conf: %s=%s : %s",
			  	ptrParams ? ptrParams : NULL,
			  	ptrVal ? ptrVal : NULL,
			  	rigerror(hamlibRigError));
		}
		ptrParams = ptrParamsNext;
	}

	int hamlibRigError = rig_open(hamlibRigPtr);
	if (RIG_OK != hamlibRigError) {
		rig_cleanup(hamlibRigPtr);
		hamlibRigPtr = NULL ;
		ThrowError("Hamlib: rig_open: %s", rigerror(hamlibRigError));
	}

	return true ;
}

void PTT::hamlibSetPtt(ptt_t aPtt)
{
	if (!hamlibRigPtr){
		ThrowError("Hamlib: Ptr shoud be allocated." );
	}
	int hamlibRigError = rig_set_ptt(hamlibRigPtr, RIG_VFO_CURR, aPtt);
	if(RIG_OK != hamlibRigError) {
		ThrowError("Hamlib: rig_set_ptt %s", rigerror(hamlibRigError) );
	}
};

void PTT::hamlibClose(void)
{
	if (hamlibRigPtr){
		int hamlibRigError = rig_close(hamlibRigPtr);
		if(RIG_OK != hamlibRigError) {
			ThrowError("Hamlib: rig_close: %s", rigerror(hamlibRigError) );
		}
		hamlibRigError = rig_cleanup(hamlibRigPtr);
		if(RIG_OK != hamlibRigError) {
			ThrowError("Hamlib: rig_cleanup: %s", rigerror(hamlibRigError) );
		}
		hamlibRigPtr = NULL ;
	}
};
#endif

PTT::~PTT(void)
{
#ifdef HAVE_LIBHAMLIB
	hamlibClose();
#else
	device.close();
#endif
}

void PTT::set(void)
{
	Config& c=Config::instance();
	if(c.readBoolEntry("/hamfax/PTT/use")) {
#ifdef HAVE_LIBHAMLIB
		if (hamlibInit()){
			hamlibSetPtt( RIG_PTT_ON );
			return ;
		}
#endif
		try {
			device.setName(c.readEntry("/hamfax/PTT/device"));
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
#ifdef HAVE_LIBHAMLIB
	if (hamlibInit()){
		hamlibSetPtt( RIG_PTT_OFF );
		return ;
	};
#endif
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
