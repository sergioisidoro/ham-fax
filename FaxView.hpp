// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <cschmit@suse.de>
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

#ifndef FAXVIEW_HPP
#define FAXVIEW_HPP

#include <qscrollview.h>
#include <qwidget.h>
#include <qpoint.h>
#include "FaxImage.hpp"

class FaxView : public QScrollView {
	Q_OBJECT
public:
	FaxView(QWidget* parent, FaxImage* faxImage);
private:
	void drawContents(QPainter* p,
			  int clipx,int clipy,int clipw,int cliph);
	virtual void contentsMousePressEvent(QMouseEvent* m);
	FaxImage* faxImage;
signals:
	void clicked(const QPoint& p);
	void clicked(void);
public slots:
        void updateView(unsigned int, unsigned int);
	void update(int x, int y, int w, int h);
};

#endif
