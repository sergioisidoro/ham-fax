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

#ifndef FAXDEMODULATOR_HPP
#define FAXDEMODULATOR_HPP

#include <qobject.h>
#include "FirFilter.hpp"
#include "LookUpTable.hpp"

/**
 * AM and FM demodulator. The demodulator takes the raw stream from
 * the sound device as input and outputs the black/white signal.
 *
 * \htmlonly <pre> \htmlonly
 * \verbatim
 *                                    I_t       I_{t-1}
 *             +----------+  +-------+     +----+  +---+
 *          +->|*sin(f_c) |->|FIR-LPF|--*->|z^-1|->|mul|--+
 *          |  +----------+  +-------+  |  +----+  +---+  |
 *          |                           *-->--     /      |
 *          |                           |     \   /       |+  y_{t-1}
 *          |                  +-------------+ \ /      +---+   +----+
 * <input>--+                  |normalization|  X       |add|-->|asin|--<out>
 *          |                  +-------------+ / \      +---+   +----+       
 *          |                           |     /   \       |-
 *          |                           *-->--     \      |
 *          |  +----------+  +-------+  |  +----+  +---+  |
 *          +->|*cos(f_c )|->|FIR-LPF|--*->|z^-1|->|mul|--+
 *             +----------+  +-------+     +----+  +---+
 *                                    Q_t       Q_{t-1}
 * \endverbatim
 *
 * \f[ y_{t-1} = I_{t-1} Q_t - Q_{t-1} I_t \f]
 * \f[ y_{t-1} = sin(2\pi ft - 2\pi f) \f]
 */

class FaxDemodulator : public QObject {
	Q_OBJECT
public:
	FaxDemodulator(QObject* parent);
        void init(int sampleRate);
private:
	typedef FirFilter<double> LPF;
	int rate;
	int deviation;
	bool fm;
	LPF iLpf;
	LPF qLpf;
	LookUpTable<double> sine;
	LookUpTable<double> cosine;
	LookUpTable<double> arcSine;
	double ifirold;
	double qfirold;
	std::valarray<double> lowPassFilter[3];
public slots:
	void newSamples(short* audio, int n);
signals:
        void data(int*, int);
};

#endif
