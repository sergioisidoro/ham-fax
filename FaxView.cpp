// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt <cschmit@suse.de>
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

#include "FaxView.hpp"
#include <qfiledialog.h>

FaxView::FaxView(QWidget* parent, FaxImage* faxImage)
	: QScrollView(parent),faxImage(faxImage)
{
	setResizePolicy(Manual);
	resizeContents(faxImage->image.width(),
		       faxImage->image.height());
}

void FaxView::drawContents(QPainter* p,
			   int clipx,int clipy,int clipw,int cliph)
{
	p->drawImage(clipx,clipy,faxImage->image,clipx,clipy,clipw,cliph);
}

void FaxView::updateView(unsigned int width, unsigned height)
{
	resizeContents(width,height);
	viewport()->update();
}

void FaxView::update(int x, int y, int w, int h)
{
	updateContents(x,y,w,h);
}
