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

#ifndef SOUND_HPP
#define SOUND_HPP

#include <qobject.h>
#include <qstring.h>

class Sound : public QObject {
	Q_OBJECT
public:
	Sound(QObject* parent);
	~Sound(void);
	void setDSPDevice(QString s);
	QString& getDSPDevice(void);
	void openOutput(unsigned int sampleRate);
	void openInput(unsigned int sampleRate);
	void close(void);
	void write(signed short* samples, unsigned int number);
	void read(signed short* samples, unsigned int& number);
	bool outputBufferEmpty(void);
private:
	QString devDSPName;
	int devDSP;
};

#endif
