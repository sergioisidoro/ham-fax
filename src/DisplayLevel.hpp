// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
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

#ifndef DISPLAYLEVEL_HPP
#define DISPLAYLEVEL_HPP

#include <qwidget.h>
#include <Q3Frame>

class DisplayLevel : public Q3Frame {
	Q_OBJECT
public:
	DisplayLevel(QWidget* parent);
	void setZero(void);
private:
	virtual void paintEvent(QPaintEvent* e);
	int m;
	int w;
public slots:
        void samples(short* buffer, int n);
};

#endif
