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
	value["autoScroll"]="1";
	value["color"]="0";
	value["toolTip"]="1";
	value["ptcSpeed"]="57600";
	value["filter"]="1";
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
				   key=="fm" ||
				   key=="autoScroll" ||
				   key=="color" ||
				   key=="toolTip" ||
				   key=="ptcSpeed" ||
				   key=="filter") {
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
	emit autoScroll(value["autoScroll"]=="1" ? true : false);
	emit color(value["color"]=="1" ? true : false);
	emit toolTip(value["toolTip"]=="1" ? true : false);
	emit ptcSpeed(value["ptcSpeed"].toUInt());
	emit filter(value["filter"].toInt());
}

Config::~Config(void)
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

bool Config::getKeyPTT(void)
{
	return value["keyPTT"]=="1";
}

void Config::setPTC(const QString& s)
{
	if(value["PTC"]!=s) {
		value["PTC"]=s;
		emit PTCDevice(s);
	}
}

void Config::setPTT(const QString& s)
{
	if(value["PTT"]!=s) {
		value["PTT"]=s;
		emit PTTDevice(s);
	}
}

void Config::setDSP(const QString& s)
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

void Config::setCarrier(int c)
{
	if(value["carrier"]!=c) {
		value["carrier"]=QString().sprintf("%d",c);
		emit carrier(c);
	}
}

void Config::setDeviation(int d)
{
	if(value["deviation"]!=d) {
		value["deviation"]=QString().sprintf("%d",d);
		emit deviation(d);
	}
}

void Config::setLpm(int l)
{
	if(value["lpm"]!=l) {
		value["lpm"]=QString().sprintf("%d",l);
		emit lpm(l);
	}
}

void Config::setAptStartFreq(int f)
{
	if(value["aptStartFreq"]!=f) {
		value["aptStartFreq"]=QString().sprintf("%d",f);;
		emit aptStartFreq(f);
	}
}

void Config::setAptStartLength(int l)
{
	if(value["aptStartLength"]!=l) {
		value["aptStartLength"]=QString().sprintf("%d",l);
		emit aptStartLength(l);
	}
}

void Config::setAptStopFreq(int f)
{
	if(value["aptStopFreq"]!=f) {
		value["aptStopFreq"]=QString().sprintf("%d",f);
		emit aptStopFreq(f);
	}
}

void Config::setAptStopLength(int l)
{
	if(value["aptStopLength"]!=l) {
		value["aptStopLength"]=QString().sprintf("%d",l);
		emit aptStopLength(l);
	}
}

void Config::setPhaseLines(int n)
{
	if(value["phaseLines"]!=n) {
		value["phaseLines"]=QString().sprintf("%d",n);
		emit phaseLines(n);
	}
}

void Config::setPhaseInvert(bool b)
{
	if(value["phaseInvert"]!=b) {
		value["phaseInvert"]=QString().sprintf("%u",b);
		emit phaseInvert(b);
	}
}

void Config::setPhaseInvert(int i)
{
	setPhaseInvert(i==1 ? true : false);
}

void Config::setUseFM(bool b)
{
	if(value["fm"]!=b) {
		value["fm"]=QString().sprintf("%u",b);
		emit useFM(b);
	}
}

void Config::setUseFM(int i)
{
	setUseFM(i==1 ? true : false);
}

void Config::setAutoScroll(bool b)
{
	if(value["autoScroll"]!=b) {
		value["autoScroll"]=QString().sprintf("%u",b);
		emit autoScroll(b);
	}
}

void Config::setColor(bool b)
{
	if(value["color"]!=b) {
		value["color"]=QString().sprintf("%u",b);
		emit color(b);
	}
}

void Config::setColor(int i)
{
	setColor(i==1 ? true : false);
}

void Config::setToolTip(bool b)
{
	if(value["toolTip"]!=b) {
		value["toolTip"]=QString().sprintf("%u",b);
	}
}

void Config::setPtcSpeed(int s)
{
	if(value["ptcSpeed"]!=s) {
		value["ptcSpeed"]=QString().sprintf("%d",s);
		emit ptcSpeed(s);
	}
}

void Config::setFilter(int n)
{
	if(value["filter"]!=n) {
		value["filter"]=QString().sprintf("%d",n);
		emit filter(n);
	}
}
