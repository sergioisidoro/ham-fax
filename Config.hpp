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

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class Config : public QObject {
	Q_OBJECT
public:
	Config(QObject* parent);
	void readFile(void);
	void writeFile(void);
	QString getDSPDevice(void);
	QString getPTTDevice(void);
	QString getPTCDevice(void);
	bool getKeyPTT(void);
private:
	typedef QMap<QString,QString> ValueMap;
	ValueMap value;
	QString fileName;
signals:
	void PTCDevice(QString s);
	void PTTDevice(QString s);
	void DSPDevice(QString s);
	void keyPTT(bool b);
	void carrier(int c);
	void deviation(int d);
	void lpm(int l);
	void aptStartFreq(int f);
	void aptStartLength(int l);
	void aptStopFreq(int f);
	void aptStopLength(int l);
	void phaseLines(int n);
	void phaseInvert(bool b);
	void useFM(bool b);
	void autoScroll(bool b);
	void color(bool b);
public slots:
        void setPTC(QString s);
	void setPTT(QString s);
	void setDSP(QString s);
	void setKeyPTT(bool b);
	void setCarrier(int c);
	void setDeviation(int d);
	void setLpm(int l);
	void setAptStartFreq(int f);
	void setAptStartLength(int l);
	void setAptStopFreq(int f);
	void setAptStopLength(int l);
	void setPhaseLines(int n);
	void setPhaseInvert(bool b);
	void setPhaseInvert(int i);
	void setUseFM(bool b);
	void setUseFM(int i);
	void setAutoScroll(bool b);
	void setColor(bool b);
	void setColor(int i);
};

#endif
