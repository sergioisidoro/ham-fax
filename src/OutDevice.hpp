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

#ifndef OUTDEVICE_HPP
#define OUTDEVICE_HPP

#include <valarray>

/**
 * OutDevice is the public interface to all possible output devices.
 */

class OutDevice {
public:
	enum Modulation { AM, FM };
	
	/**
	 * Open the file and return the file descriptor of the device.
	 * This is intended for using select() on the device.
	 * \param fileName is the name of the file or device to open
	 * \param mod specifies the modulation type
	 * \param sampleRate is the desired sample rate; note that the real rate 
	 * can be different.
	 */
	virtual int open(const char* fileName, const Modulation mod,
			 const unsigned int sampleRate)=0;

	/**
	 * Return the sample rate being used.
	 */
	virtual unsigned int sampleRate()=0;

	/**
	 * Write a block of data to device.
	 * 0 stands for black and 255 for white, hence an unsigned char is ok
	 */
	virtual void transmit(std::valarray<unsigned char>)=0;

	/**
	 * Return output delay (maximum number of samples in out buffer)
	 */
	virtual unsigned int getOutputDelay()=0;

	/**
	 * Close the device.
	 */
	virtual void close()=0;

	/**
	 * Virtual destructor for correct destruction.
	 */
	virtual ~OutDevice();
};

#endif
