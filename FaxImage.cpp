// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmittt, DH1CS <cschmit@suse.de>
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

FaxImage::FaxImage(QObject* parent)
	: QObject(parent)
{
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
	return qGray(image.pixel(col,row));
}

unsigned int FaxImage::getPixelGreen(unsigned int col, unsigned int row)
{
	return qGray(image.pixel(col,row));
}

unsigned int FaxImage::getPixelBlue(unsigned int col, unsigned int row)
{
	return qGray(image.pixel(col,row));
}

bool FaxImage::setPixelGray(unsigned int col, unsigned int row,
				   unsigned int value)
{
	if(col<=(unsigned int)image.width() && 
	   row<(unsigned int)image.height()) {
		image.setPixel(col,row,qRgb(value,value,value));
		emit contentUpdated(col,row,1,1);
		return true;
	}
	return false;
}

void FaxImage::setPixelRed(unsigned int col, unsigned int row,
				  unsigned int value)
{
	image.setPixel(col,row,qRgb(value,
				    getPixelBlue(col,row),
				    getPixelGreen(col,row)));
	emit contentUpdated(col,row,1,1);
}

void FaxImage::setPixelGreen(unsigned int col, unsigned int row,
				    unsigned int value)
{
	image.setPixel(col,row,qRgb(getPixelRed(col,row),
				    value,
				    getPixelBlue(col,row)));
	emit contentUpdated(col,row,1,1);
}

void FaxImage::setPixelBlue(unsigned int col, unsigned int row,
				   unsigned int value)
{
	image.setPixel(col,row,qRgb(getPixelRed(col,row),
				    getPixelGreen(col,row),
				    value));
	emit contentUpdated(col,row,1,1);
}

void FaxImage::create(unsigned int cols, unsigned int rows)
{
	image.create(cols,rows,32);
	image.fill(qRgb(80,80,80));
	emit sizeUpdated(image.width(),image.height());
	emit widthUpdated(image.width());
}

void FaxImage::load(QString fileName)
{
	image=QImage(fileName,"PNG").convertDepth(32);
	emit sizeUpdated(image.width(),image.height());
	emit widthUpdated(image.width());
}

void FaxImage::save(QString fileName)
{
	image.save(fileName,"PNG");
}

void FaxImage::scale(unsigned int width, unsigned int height)
{
	image=image.smoothScale(width,height);
	emit sizeUpdated(image.width(),image.height());
	emit widthUpdated(image.width());
}

void FaxImage::resize(unsigned int x, unsigned int y,
		      unsigned int w, unsigned int h)
{
	image=image.copy(x,y,w,h);
	emit sizeUpdated(image.width(),image.height());
	emit widthUpdated(image.width());
}
