// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2011 Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
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

#ifndef IMAGEWIDGET_HPP
#define IMAGEWIDGET_HPP

#include <QImage>
#include <QWidget>

/**
 * Widget that paints itself according to the contents of a QImage. This class
 * is used for displaying the image inside the scroll area.
 */
class ImageWidget: public QWidget {
	Q_OBJECT
public:
	ImageWidget(const QImage& image);
private:
	const QImage& image;

	/**
	 * Paint the area on the screen with the contents from the QImage.
	 */
	virtual void paintEvent(QPaintEvent* event);
};

#endif
