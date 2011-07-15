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

#include "Spectrum.hpp"
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <cmath>
#include <cstring>

Spectrum::Spectrum(QWidget* parent)
	: QFrame(parent), margin(2)
{
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
	setContentsMargins(margin, margin, margin, margin);
	setFixedSize(260 + 2 * margin, 126 + 2 * margin);
	pixmap=new QPixmap(width() - 2 * margin, height() - 2 * margin);
	QPainter paint(pixmap);
	paint.eraseRect(0,0,width(),height());
}

void Spectrum::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);
	QPainter painter(this);
	painter.drawPixmap(margin, margin, *pixmap,
			   0, 0, width() - 2 * margin, height() - 2 * margin);
}


static void draw_spectrum_line(int y, QPainter& paint,
			       const double *const data, const int buf_size)
{
	int from_x = 2;
	int from_y = y - 1;

	for (int i = 0; i < buf_size; i++) {
		int to_y = y - data[i];
		int to_x = i + 2;

		paint.drawLine(from_x, from_y, to_x , to_y);
		from_x = to_x;
		from_y = to_y;
	}
}

static void draw_scale_lines(int step, int y, QPainter& paint)
{
	for (int x = 2; x < 254; x += step)
		paint.drawLine(x, 95, x, y);
}

void Spectrum::samples(int* buffer, int n)
{
	const int buf_size = 256;
	double data[buf_size]; // spectrum from 0 (black) to buf_size (white)

	std::memset(data, 0, sizeof(data));

	// fill buffer with histogram data
	for (int i = 0; i<n; i++) {
		data[buffer[i]]++;
	}

	// normalize data
	for (int i = 0; i < buf_size; i++) {
		data[i] = 1024.0 * data[i] / n;
		data[i] = std::min(data[i], 93.0);
		data[i] = std::max(data[i], 0.0);
	}

	QPainter paint(pixmap);
	paint.eraseRect(0, 0, width(),height());
	paint.setPen(Qt::black);

	// draw scale "black to white"
	paint.drawLine(2, 95, 254, 95);
	paint.drawText(6, 115, tr("B"));
	paint.drawText(236, 115, tr("W"));
	draw_scale_lines(25, 104, paint);
	draw_scale_lines(5, 99, paint);

	// and finally the spectrum itself
	draw_spectrum_line(95, paint, data, buf_size);;
	paint.setPen(Qt::gray);
	draw_spectrum_line(94, paint, data, buf_size);

	update();
}
