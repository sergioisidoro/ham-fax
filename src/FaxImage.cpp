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

#include "FaxImage.hpp"
#include "Config.hpp"
#include "ImageWidget.hpp"
#include <QMouseEvent>
#include <QImageWriter>

FaxImage::FaxImage(QWidget* parent)
	: QScrollArea(parent)
{
	autoScroll=Config::instance().readBoolEntry("/hamfax/GUI/autoScroll");
	setWidget(new ImageWidget(image));
}

int FaxImage::getRows(void)
{
	return image.height();
}

int FaxImage::getCols(void)
{
	return image.width();
}

int FaxImage::getPixel(int col, int row, int rgbg)
{
	QRgb pixel=image.pixel(col,row);
	int value;
	switch(rgbg) {
	case 0: 
		value=qRed(pixel);
		break;
	case 1: 
		value=qGreen(pixel);
		break;
	case 2: 
		value=qBlue(pixel);
		break;
	default:
		value=qGray(pixel);
		break;
	};
	return value;
}

bool FaxImage::setPixel(int col, int row, int value, int rgbg)
{
	if(col>=image.width() || row>=image.height()+1) {
		return false;
	}
	if(row>=image.height()) {
		resizeHeight(50);
	}
	QRgb oldColor=image.pixel(col,row);
	QRgb color;
	switch(rgbg) {
	case 0:
		color=qRgb(value,qGreen(oldColor),qBlue(oldColor));
		break;
	case 1:
		color=qRgb(qRed(oldColor),value,qBlue(oldColor));
		break;
	case 2:
		color=qRgb(qRed(oldColor),qGreen(oldColor),value);
		break;
	default:
		color=qRgb(value,value,value);
		break;
	};
	image.setPixel(col,row,color);
	widget()->update(col, row, 1, 1);
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
		widget()->resize(imageW, imageH + h);
		if(h>0) {
			widget()->update(0, imageH, imageW, imageH + h);
		}
		if(h<0) {
			widget()->update(0, imageH + h, imageW, imageH);
		}
		emit sizeUpdated(imageW,imageH+h);
	}
}

void FaxImage::create(int cols, int rows)
{
	image.create(cols,rows,32);
	image.fill(qRgb(80,80,80));
	widget()->resize(cols, rows);
	widget()->update(0, 0, cols, rows);
	emit sizeUpdated(cols,rows);
	emit newImage();
}

void FaxImage::load(QString fileName)
{
	image=QImage(fileName).convertDepth(32);
	widget()->resize(image.width(), image.height());
	widget()->update(0, 0, image.width(), image.height());
	emit sizeUpdated(image.width(),image.height());
	emit newImage();
}

bool FaxImage::save(QString fileName)
{
	QString handler;
	int n = fileName.findRev('.');

	if(n != -1) {
		QString ext = fileName.right(fileName.length() - n - 1).upper();
		if(QImageWriter::supportedImageFormats().contains(ext.toAscii()))
			handler = ext;
	}

	// No valid extension in file name. Try PNG as default.
	fileName.append(".png");
	handler = "PNG";

	return image.save(fileName,handler);
}

void FaxImage::scale(int width, int height)
{
	image=image.smoothScale(width,height);
	widget()->resize(width, height);
	widget()->update(0, 0, width, height);
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
	widget()->resize(w,h);
	widget()->update(0,0,w,h);
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
	emit widthAdjust(static_cast<double>
			 (slant2.x()-slant1.x())/(slant1.y()-slant2.y())
			 /image.width());
}

void FaxImage::mousePressEvent(QMouseEvent* m)
{
	QPoint pos = widget()->mapFromParent(m->pos());

	slant1 = slant2;
	slant2 = pos;
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
	widget()->update(0, 0, w, h);
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
	widget()->update(0, 0, w, h);
	emit newImage();
}
