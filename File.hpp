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

#ifndef FILE_HPP
#define FILE_HPP

#include <qobject.h>
#include <audiofile.h>

class File : public QObject {
	Q_OBJECT
public:
	File(QObject* parent);
	~File(void);
	void openOutput(const QString& fileName,unsigned int sampleRate);
	void openInput(const QString& fileName,unsigned int& sampleRate);
	void write(signed short* samples, unsigned int number);
	void read(signed short* samples, unsigned int& number);
	void close(void);
private:
	AFfilehandle aFile;
};

#endif