// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <christof.schmitt@gmx.de>
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

#include "Spectrum.hpp"
#include <qpainter.h>
#include <qpixmap.h>
#include <math.h>

Spectrum::Spectrum(QWidget* parent)
	: QFrame(parent)
{
	setFrameStyle(QFrame::Panel|QFrame::Sunken);
	setMargin(2);
	m=margin();
	setFixedSize(128+2*m,64+2*m);
	w=width();
	h=height();
	pixmap=new QPixmap(w-2*m,h-2*m);
	QPainter paint(pixmap,this);
	paint.eraseRect(0,0,w,h);
	paint.flush();
}

void Spectrum::init(void)
{
	for(int i=0; i<64; i++) {
		data[i]=0;
	}
}

void Spectrum::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
	bitBlt(this,m,m,pixmap,0,0,w-2*m,h-2*m);
}

void Spectrum::samples(int* buffer, int n)
{
	for(int i=0; i<64; i++) {
		data[i]=0;
	}
	for(int i=0; i<n; i++) {
		data[buffer[i]/4]++;
	}
	for(int i=0; i<64; i++) {
		data[i]=data[i]*128.0/n+1;
		data[i]=log(data[i])*32.0;
		if(data[i]>127) {
			data[i]=127;
		}
	}
	QPainter paint(pixmap,this);
	paint.eraseRect(0,0,w,h);
	paint.setPen(Qt::blue);
	paint.setBrush(Qt::blue);
	for(int i=0; i<64; i++) {
		paint.drawRect(i*2,128-data[i],2,data[i]);
	}
	paint.flush();
	update();
}
