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

#ifndef FILE_HPP
#define FILE_HPP

#include <qobject.h>
#include <audiofile.h>
#include <qtimer.h>

class File : public QObject {
	Q_OBJECT
public:
	File(QObject* parent);
	~File(void);
	void startOutput(const QString& fileName);
	void startInput(const QString& fileName);
	void read(signed short* samples, unsigned int& number);
	void end(void);
private:
	int sampleRate;
	AFfilehandle aFile;
	QTimer* timer;
signals:
	void newSampleRate(int);
	void data(signed short* buffer, unsigned int n);
	void next(unsigned int n);
public slots:
	void write(signed short* samples, unsigned int number);
	void timerSignal(void);
private slots:
        void read(void);
};

#endif
