// hamfax -- an application for sending and receiving amateur radio facsimiles
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

/**
 * \file
 *
 * PTC definition. This class implements the methods for accessing the SCS-PTC.
 */

#ifndef PTC_HPP
#define PTC_HPP

#include <qobject.h>
#include <qstring.h>
#include <qsocketnotifier.h>

/**
 * SCS-PTC interface. Transmitting and receiving via the SCS-PTC is implemented here.
 *
 * The device uses the usual RS232 serial interface for transferring
 * the unmodulated data. The modulation and demodulation happens in
 * the PTC's DSP. Manuals are available online:
 * http://www.scs-ptc.com/software.html#manuals
 *
 * - 115200, 57600 or 38400 bit/s
 * - FAX MBAUD set to same bit rate
 * - FAX DEVIATION (default 400Hz)
 * - FM reception with default deviation: 1500Hz=0, 1900Hz=128, 2300Hz=255
 * - AM reception: Input range 0 to 255
 * - Transmitting: Output range 0 to 63, data rate 1/20 MBaud rate
 * - Sending 255(dec)=377(oct) exits transmitting mode
 * - Reception: Input data rate 1/20 MBaud rate
 */
 
class PTC : public QObject {
	Q_OBJECT
public:
	PTC(QObject* parent);
	~PTC(void);
	int startInput(void);
	int startOutput(void);
	void end(void);
private:
	void open(void);
	int device;
	int speed;
	bool fm;
	QSocketNotifier* notifier;
signals:
	void data(int*, int);
	void spaceLeft(int);
	void deviceClosed();
public slots:
	void transmit(double* samples, int count);
private slots:
        void read(int fd);
	void checkSpace(int fd);
	void close(void);
};

#endif
