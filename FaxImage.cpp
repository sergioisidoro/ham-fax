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
	: QScrollView(parent,0,WResizeNoErase|WNorthWestGravity),
	  autoScroll(true)
{
	setResizePolicy(Manual);
	viewport()->setBackgroundMode(PaletteMid);
}

int FaxImage::getRows(void)
{
	return image.height();
}

int FaxImage::getCols(void)
{
	return image.width();
}

int FaxImage::getPixelGray(int col, int row)
{
	return qGray(image.pixel(col,row));
}

int FaxImage::getPixelRed(int col, int row)
{
	return qRed(image.pixel(col,row));
}

int FaxImage::getPixelGreen(int col, int row)
{
	return qGreen(image.pixel(col,row));
}

int FaxImage::getPixelBlue(int col, int row)
{
	return qBlue(image.pixel(col,row));
}

bool FaxImage::setPixel(int col, int row, int value, int rgbg)
{
	if(col>=image.width() || row>=image.height()+1) {
		return false;
	}
	if(row>=image.height()) {
		resizeHeight(50);
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

void FaxImage::resizeHeight(int h)
{
	int imageW=image.width();
	int imageH=image.height();
	image=image.copy(0,0,imageW,imageH+h);
	if(imageH+h>=1) {
		resizeContents(imageW,imageH+h);
		if(h>0) {
			updateContents(0,imageH,imageW,imageH+h);
		}
		if(h<0) {
			updateContents(0,imageH+h,imageW,imageH);
		}
		emit sizeUpdated(imageW,imageH+h);
	}
}

void FaxImage::create(int cols, int rows)
{
	image.create(cols,rows,32);
	image.fill(qRgb(80,80,80));
	resizeContents(cols,rows);
	updateContents(0,0,cols,rows);
	emit sizeUpdated(cols,rows);
	emit newImage();
}

void FaxImage::load(QString fileName)
{
	image=QImage(fileName).convertDepth(32);
	resizeContents(image.width(),image.height());
	updateContents(0,0,image.width(),image.height());
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

void FaxImage::scale(int width, int height)
{
	image=image.smoothScale(width,height);
	resizeContents(width,height);
	updateContents(0,0,width,height);
	emit sizeUpdated(width,height);
	emit newImage();
}

void FaxImage::scale(int width)
{
	scale(width,width*image.height()/image.width());
}

void FaxImage::resize(int x, int y, int w, int h)
{
	if(w==0) {
		w=image.width();
	}
	if(h==0) {
		h=image.height();
	}
	image=image.copy(x, y, w, h);
	resizeContents(w,h);
	updateContents(0,0,w,h);
	emit sizeUpdated(w,h);
}

void FaxImage::setWidth(int w)
{
	scale(w,image.height());
}

void FaxImage::setAutoScroll(bool b)
{
	autoScroll=b;
}

void FaxImage::correctSlant(void)
{
	emit widthAdjust(static_cast<double>(slant2.x()-slant1.x())
			 /(slant1.y()-slant2.y())/image.width());
}

void FaxImage::drawContents(QPainter* p,int x,int y,int w,int h)
{
	p->drawImage(x,y,image,x,y,w,h);
}

void FaxImage::contentsMousePressEvent(QMouseEvent* m)
{
	slant1=slant2;
	slant2=m->pos();
	emit clicked();
}

void FaxImage::shiftColors(void)
{
	int w=image.width();
	int h=image.height();
	for(int c=0; c<w; c++) {
		for(int r=0; r<h; r++) {
			QRgb rgb=image.pixel(c,r);
			rgb=qRgb(qGreen(rgb),qBlue(rgb),qRed(rgb));
			image.setPixel(c,r,rgb);
		}
	}
	updateContents(0,0,w,h);
	emit newImage();
}

void FaxImage::correctBegin(void)
{
	int n=slant2.x();
	int h=image.height();
	int w=image.width();
	QImage tempImage(w,h,32);
	for(int c=0; c<w; c++) {
		for(int r=0; r<h; r++) {
			tempImage.setPixel(c,r,image.pixel((n+c)%w,r));
		}
	}
	image=tempImage;
	updateContents(0,0,w,h);
	emit newImage();
}
