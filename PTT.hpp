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

#ifndef PTT_HPP
#define PTT_HPP

#include <qstring.h>
#include <qobject.h>

class PTT : public QObject {
	Q_OBJECT
public:
	PTT(QObject* parent);
	~PTT(void);
	void setDeviceName(QString s);
	QString& getDeviceName(void);
	void openDevice(void);
	void closeDevice(void);
	void set(bool press);
private:
	QString deviceName;
	int device;
	bool usePTT;
public slots:
        void setUse(bool use);
signals:
        void newPTTUsage(bool use);
};

#endif
