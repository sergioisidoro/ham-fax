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

#ifndef PTC_HPP
#define PTC_HPP

#include <qobject.h>
#include <qstring.h>
#include <qsocketnotifier.h>

class PTC : public QObject {
	Q_OBJECT
public:
	PTC(QObject* parent);
	~PTC(void);
	void startInput(void);
	void startOutput(void);
	void end(void);
private:
	void open(void);
	QString deviceName;
	int device;
	bool fm;
	int speed;
	int deviation;
	QSocketNotifier* notifier;
signals:
	void newSampleRate(int);
	void data(int*, int);
	void spaceLeft(int);
	void deviceClosed();
public slots:
        void setDeviceName(const QString& s);
        void setDeviation(int dev);
	void setSpeed(int s);
	void setFM(bool fm);
	void transmit(double* samples, int count);
private slots:
        void read(int fd);
	void checkSpace(int fd);
	void close(void);
};

#endif
