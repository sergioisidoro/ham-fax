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

#ifndef FAXMODULATOR_HPP
#define FAXMODULATOR_HPP

#include <qobject.h>
#include "LookUpTable.hpp"

/**
 * Create modulated signal. This class creates the modulated FM or AM
 * signal, ready for transmitting.
 *
 * \todo The Qt signal and slot mechanism does not belong here.
 */
class FaxModulator : public QObject {
	Q_OBJECT
public:
	/**
	 * Constructor.
         *
	 * \param parent is either a pointer to the Qt parent window or 0
	 */
	FaxModulator(QObject* parent);
    
        /**
	 * Initialize everything to get ready for transmission.
         *
	 * \param sampleRate sets the sample rate for the transmission
	 */
        void init(int sampleRate);
private:
	int sampleRate;
	bool fm;
	int carrier;
	int dev;
	LookUpTable<short> sine;
signals:
	/**
	 * The signal is emitted with an array holding the modulated signal 
	 * and the array's length.
	 */
	void data(short*, int);
public slots:
	/**
	 * \param buffer is an array of the length
	 * \param n and holds the signal to be modulated.
	 * After that the signal data is emitted.
	 */
	void modulate(double* buffer, int n);
};

#endif
