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

#include "Config.hpp"
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>

Config::Config(QObject* parent)
	: QObject(parent)
{
	value["PTC"]="/dev/ttyS0";
	value["PTT"]="/dev/ttyS1";
	value["DSP"]="/dev/dsp";
	value["keyPTT"]="0";
	value["carrier"]="1900";
	value["deviation"]="400";
	value["lpm"]="120";
	value["aptStartFreq"]="300";
	value["aptStartLength"]="5";
	value["aptStopFreq"]="450";
	value["aptStopLength"]="5";
	value["phaseLines"]="20";
	value["phaseInvert"]="0";
	value["fm"]="1";
}

void Config::readFile(void)
{
	fileName=QDir::homeDirPath()+"/.hamfax";
	QFile f(fileName);
	if (f.open(IO_ReadOnly)) {
		QTextStream t(&f);
		QString s;
		while(!t.eof()) {
			s=t.readLine();
			int n;
			if((n=s.find('='))!=-1) {
				QString key=s.left(n);
				QString data=s.right(s.length()-n-1);
				if(key=="PTC" ||
				   key=="PTT" ||
				   key=="DSP" ||
				   key=="keyPTT" ||
				   key=="carrier" ||
				   key=="deviation" ||
				   key=="lpm" ||
				   key=="aptStartFreq" ||
				   key=="aptStartLength" ||
				   key=="aptStopFreq" ||
				   key=="aptStopLength" ||
				   key=="phaseLines" ||
				   key=="phaseInvert" ||
				   key=="fm") {
					value[key]=data;
				}
			};
		}
		f.close();
	}
	emit PTCDevice(value["PTC"]);
	emit PTTDevice(value["PTT"]);
	emit DSPDevice(value["DSP"]);
	emit keyPTT(value["keyPTT"]=="1" ? true : false);
	emit carrier(value["carrier"].toUInt());
	emit deviation(value["deviation"].toUInt());
	emit lpm(value["lpm"].toUInt());
	emit aptStartFreq(value["aptStartFreq"].toUInt());
	emit aptStartLength(value["aptStartLength"].toUInt());
	emit aptStopFreq(value["aptStopFreq"].toUInt());
	emit aptStopLength(value["aptStopLength"].toUInt());
	emit phaseLines(value["phaseLines"].toUInt());
	emit phaseInvert(value["phaseInvert"]=="1" ? true : false);
	emit useFM(value["fm"]=="1" ? true : false);
}

void Config::writeFile(void)
{
	QFile f(QDir::homeDirPath()+"/.hamfax");
	if (f.open(IO_WriteOnly)) {
		QTextStream t(&f);
		ValueMap::Iterator it;
		for(it=value.begin(); it!=value.end(); ++it) {
			t<<it.key()<<'='<<it.data()<<endl;
		}
		f.close();
	}
}

QString Config::getDSPDevice(void)
{
	return value["DSP"];
}

QString Config::getPTTDevice(void)
{
	return value["PTT"];
}


QString Config::getPTCDevice(void)
{
	return value["PTC"];
}

bool Config::getKeyPTT(void)
{
	return value["keyPTT"]=="1";
}

void Config::setPTC(QString s)
{
	if(value["PTC"]!=s) {
		value["PTC"]=s;
		emit PTCDevice(s);
	}
}

void Config::setPTT(QString s)
{
	if(value["PTT"]!=s) {
		value["PTT"]=s;
		emit PTTDevice(s);
	}
}

void Config::setDSP(QString s)
{
	if(value["DSP"]!=s) {
		value["DSP"]=s;
		emit DSPDevice(s);
	}
}

void Config::setKeyPTT(bool b)
{
	if(value["keyPTT"]!=b) {
		value["keyPTT"]=QString().sprintf("%u",b);
		emit keyPTT(b);
	}
}

void Config::setCarrier(unsigned int c)
{
	if(value["carrier"]!=c) {
		value["carrier"]=QString().sprintf("%u",c);
		emit carrier(c);
	}
}

void Config::setDeviation(unsigned int d)
{
	if(value["deviation"]!=d) {
		value["deviation"]=QString().sprintf("%u",d);
		emit deviation(d);
	}
}

void Config::setLpm(unsigned int l)
{
	if(value["lpm"]!=l) {
		value["lpm"]=QString().sprintf("%u",l);
		emit lpm(l);
	}
}

void Config::setAptStartFreq(unsigned int f)
{
	if(value["aptStartFreq"]!=f) {
		value["aptStartFreq"]=QString().sprintf("%u",f);;
		emit aptStartFreq(f);
	}
}

void Config::setAptStartLength(unsigned int l)
{
	if(value["aptStartLength"]!=l) {
		value["aptStartLength"]=QString().sprintf("%u",l);
		emit aptStartLength(l);
	}
}

void Config::setAptStopFreq(unsigned int f)
{
	if(value["aptStopFreq"]!=f) {
		value["aptStopFreq"]=QString().sprintf("%u",f);
		emit aptStopFreq(f);
	}
}

void Config::setAptStopLength(unsigned int l)
{
	if(value["aptStopLength"]!=l) {
		value["aptStopLength"]=QString().sprintf("%u",l);
		emit aptStopLength(l);
	}
}

void Config::setPhaseLines(unsigned int n)
{
	if(value["phaseLines"]!=n) {
		value["phaseLines"]=QString().sprintf("%u",n);
		emit phaseLines(n);
	}
}

void Config::setPhaseInvert(bool b)
{
	if(value["phaseInvert"]!=b) {
		value["phaseInvert"]=QString().sprintf("%u",(int)b);
		emit phaseInvert(b);
	}
}

void Config::setUseFM(bool b)
{
	if(value["fm"]!=b) {
		value["fm"]=QString().sprintf("%u",(int)b);
		emit useFM(b);
	}
}
