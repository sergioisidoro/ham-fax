// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2002
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

#ifndef DEVICEFACTORY_HPP
#define DEVICEFACTORY_HPP

#include <memory>
#include "InDevice.hpp"
#include "OutDevice.hpp"

/**
 * DeviceFactory is the only (therefore a Singleton) place for creating 
 * input and output objects. Just call the appropiate method, use the new object
 * and don't forget to delete it: It is yours.
 */

class DeviceFactory {
public:
        /**
	 * Only possible access to Singleton
	 */
	static DeviceFactory& instance() const;

	/**
	 * Available input devices.
	 */
	typedef enum { AUDIOFILE, OSS, PTC } in_t;

	/**
	 * Available output devices.
	 */
	typedef enum { AUDIOFILE, OSS, PTC } out_t;

	/**
	 * Specify the input device type you want and get it.
	 */
	InDevice* getInputDevice(in_t in) const;

	/**
	 * Specify the output device type you want and get it.
	 */
	OutDevice* getOutputDevice(out_t out) const;
private:
	typedef std::auto_ptr<DeviceFactory> DeviceFactoryPtr;
	friend class DeviceFactoryPtr;
};

#endif
