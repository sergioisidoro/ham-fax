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

class FaxModulator : public QObject {
	Q_OBJECT
public:
	FaxModulator(QObject* parent);
private:
	int sampleRate;
	bool fm;
	int carrier;
	int dev;
	LookUpTable<short> sine;
signals:
	void data(short*, int);
public slots:
        void init(void);
	void setSampleRate(int sr);
        void setCarrier(int carrier);
	void setDeviation(int dev);
	void setFM(bool fm);
	void modulate(double* buffer, int n);
};

#endif
