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

#ifndef FAXIMAGE_HPP
#define FAXIMAGE_HPP

#include <qimage.h>
#include <qpainter.h>
#include <qpoint.h>
#include <Q3ScrollView>
#include <qstring.h>

class FaxImage : public Q3ScrollView {
	Q_OBJECT
public:
	FaxImage(QWidget* parent);
	int getRows(void);
	int getCols(void);
	int getPixel(int col, int row, int rgbg);
	void load(QString fileName);
	bool save(QString fileName);
private:
	void drawContents(QPainter* p, int x,int y,int w,int h);
	virtual void contentsMousePressEvent(QMouseEvent* m);
	void resizeHeight(int h);
	QImage image;
	QPoint slant1;
	QPoint slant2;
	bool autoScroll;
signals:
	void sizeUpdated(int, int);
	void clicked(void);
	void widthAdjust(double);
	void newImage(void);
	void shiftLine(double);
public slots:
        bool setPixel(int col, int row, int value, int rgbg);
        void create(int cols, int rows);
	void scale(int width, int height);
	void scale(int width);
	void resize(int x, int y, int w, int h=0);
	void setWidth(int w);
	void setAutoScroll(bool b);
	void correctSlant(void);
	void shiftColors(void);
	void correctBegin(void);
};

#endif
