// HamFax -- an application for sending and receiving amateur radio facsimiles
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

#ifndef SOUND_HPP
#define SOUND_HPP

#include <qobject.h>
#include <qstring.h>
#include <qsocketnotifier.h>
#include "PTT.hpp"

class Sound : public QObject {
	Q_OBJECT
public:
	Sound(QObject* parent);
	~Sound(void);
	int startOutput(void);
	int startInput(void);
private:
	int sampleRate;
	int dsp;
	QSocketNotifier* notifier;
	PTT ptt;
signals:
        void data(short*, int);
	void deviceClosed(void);
	void spaceLeft(int);
public slots:
	void closeNow(void);
	void end(void);
	void write(short* samples, int number);
private slots:
        void read(int fd);
	void checkSpace(int fd);
	void close(void);
};

#endif
