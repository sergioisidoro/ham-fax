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

#ifndef FAXDEMODULATOR_HPP
#define FAXDEMODULATOR_HPP

#include <qobject.h>
#include "FirFilter.hpp"
#include "LookUpTable.hpp"

class FaxDemodulator : public QObject {
	Q_OBJECT
public:
	FaxDemodulator(QObject* parent);
        void init(int sampleRate);
private:
	int rate;
	int deviation;
	bool fm;
	FirFilter<double> iFir;
	FirFilter<double> qFir;
	LookUpTable<double> sine;
	LookUpTable<double> cosine;
	LookUpTable<double> arcSine;
	double ifirold;
	double qfirold;
public slots:
	void newSamples(short* audio, int n);
signals:
        void data(int*, int);
};

#endif
