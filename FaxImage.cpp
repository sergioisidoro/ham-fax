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

#include "FaxImage.hpp"
#include <math.h>

FaxImage::FaxImage(QWidget* parent)
	: QScrollView(parent), autoScroll(true)
{
	setResizePolicy(Manual);
}

unsigned int FaxImage::getRows(void)
{
	return image.height();
}

unsigned int FaxImage::getCols(void)
{
	return image.width();
}

unsigned int FaxImage::getPixelGray(unsigned int col, unsigned int row)
{
	return qGray(image.pixel(col,row));
}

unsigned int FaxImage::getPixelRed(unsigned int col, unsigned int row)
{
	return qRed(image.pixel(col,row));
}

unsigned int FaxImage::getPixelGreen(unsigned int col, unsigned int row)
{
	return qGreen(image.pixel(col,row));
}

unsigned int FaxImage::getPixelBlue(unsigned int col, unsigned int row)
{
	return qBlue(image.pixel(col,row));
}

bool FaxImage::setPixel(unsigned int col, unsigned int row,
			unsigned int value, unsigned int rgbg)
{
	if(col>=(unsigned int)image.width() ||
	   row>=(unsigned int)image.height()+1) {
		return false;
	}
	if(row>=(unsigned int)image.height()) {
		resize(0,0,image.width(),image.height()+50);
	}
	switch(rgbg) {
	case 0:
		image.setPixel(col,row,qRgb(value,
					    getPixelGreen(col,row),
					    getPixelBlue(col,row)));
		break;
	case 1:
		image.setPixel(col,row,qRgb(getPixelRed(col,row),
					    value,
					    getPixelBlue(col,row)));
		break;
	case 2:
		image.setPixel(col,row,qRgb(getPixelRed(col,row),
					    getPixelGreen(col,row),
					    value));
		break;
	case 3:
		image.setPixel(col,row,qRgb(value,value,value));
		break;
	};
	updateContents(col,row,1,1);
	if(autoScroll) {
		ensureVisible(0,row,0,0);
	}
	return true;
}

void FaxImage::create(unsigned int cols, unsigned int rows)
{
	image.create(cols,rows,32);
	image.fill(qRgb(80,80,80));
	resizeContents(cols,rows);
	emit sizeUpdated(cols,rows);
	emit newImage();
}

void FaxImage::load(QString fileName)
{
	image=QImage(fileName).convertDepth(32);
	resizeContents(image.width(),image.height());
	emit sizeUpdated(image.width(),image.height());
	emit newImage();
}

void FaxImage::save(QString fileName)
{
	int n=fileName.find('.');
	if(n!=-1) {
		QString handler=fileName.right(fileName.length()-n-1).upper();
		if(QImageIO::outputFormats().contains(handler)) {
			image.save(fileName,handler);
		}
	}
}

void FaxImage::scale(unsigned int width, unsigned int height)
{
	if(height==0) {
		height=width*image.height()/image.width();
	}
	image=image.smoothScale(width,height);
	resizeContents(width,height);
	emit sizeUpdated(width,height);
}

void FaxImage::resize(unsigned int x, unsigned int y,
		      unsigned int w, unsigned int h)
{
	if(w==0) {
		w=image.width();
	}
	if(h==0) {
		h=image.height();
	}
	image=image.copy(x, y, w, h);
	resizeContents(w,h);
	emit sizeUpdated(w,h);
}

void FaxImage::setAutoScroll(bool b)
{
	autoScroll=b;
}

void FaxImage::correctSlant(void)
{
	emit widthAdjust(1.0-(double)(slant1.x()-slant2.x())
			 /(double)(slant1.y()-slant2.y())
			 /(double)image.width());
}

void FaxImage::drawContents(QPainter* p,
			    int clipx,int clipy,int clipw,int cliph)
{
	p->drawImage(clipx,clipy,image,clipx,clipy,clipw,cliph);
}

void FaxImage::contentsMousePressEvent(QMouseEvent* m)
{
	slant1=slant2;
	slant2=m->pos();
	emit clicked();
}


void FaxImage::shiftCol1(void)
{
	for(int c=0; c<image.width(); c++) {
		for(int r=0; r<image.height(); r++) {
			QRgb rgb=image.pixel(c,r);
			rgb=qRgb(qGreen(rgb),qBlue(rgb),qRed(rgb));
			image.setPixel(c,r,rgb);
		}
	}
	viewport()->update();
}

void FaxImage::shiftCol2(void)
{
	for(int c=0; c<image.width(); c++) {
		for(int r=0; r<image.height(); r++) {
			QRgb rgb=image.pixel(c,r);
			rgb=qRgb(qBlue(rgb),qRed(rgb),qGreen(rgb));
			image.setPixel(c,r,rgb);
		}
	}
	viewport()->update();
}

void FaxImage::correctBegin(void)
{
	int n=slant2.x();
	int h=image.height();
	int w=image.width();
	for(int c=0; c<w; c++) {
		for(int r=0; r<h; r++) {
			QRgb pixel;
			if(c+n<w) {
				pixel=image.pixel(n+c,r);
			} else if(r+1<h) {
				pixel=image.pixel((n+c)%w,r+1);
			} else {
				pixel=qRgb(0,0,0);
			}
			image.setPixel(c,r,pixel);
		}
	}
	viewport()->update();
}
