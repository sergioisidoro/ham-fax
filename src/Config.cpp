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

#include "Config.hpp"
#include <qdir.h>

Config& Config::instance()
{
	static ConfigPtr i(new Config);
	return *i;
}

Config::Config()
{
	insertSearchPath( QSettings::Unix, "/etc" );

	setDefault("/hamfax/sound/device","/dev/dsp");
	setDefault("/hamfax/PTC/device","/dev/ttyS0");
	setDefault("/hamfax/PTC/speed",38400);
	setDefault("/hamfax/PTT/device","/dev/ttyS1");
	setDefault("/hamfax/PTT/use",false);
	setDefault("/hamfax/APT/startLength",5);
	setDefault("/hamfax/APT/startFrequency",300);
	setDefault("/hamfax/APT/stopLength",5);
	setDefault("/hamfax/APT/stopFrequency",450);
	setDefault("/hamfax/modulation/carrier",1900);
	setDefault("/hamfax/modulation/deviation",400);
	setDefault("/hamfax/modulation/filter",1);
	setDefault("/hamfax/modulation/FM",true);
	setDefault("/hamfax/fax/color",false);
	setDefault("/hamfax/fax/LPM",120);
	setDefault("/hamfax/phasing/lines",20);
	setDefault("/hamfax/phasing/invert",false);
	setDefault("/hamfax/directories/qm", PKGDATADIR);
	setDefault("/hamfax/directories/doc",PKGDATADIR);
	setDefault("/hamfax/GUI/toolTips",true);
	setDefault("/hamfax/GUI/autoScroll",true);
	setDefault("/hamfax/GUI/font","Helvetica,11,-1,5,50,0,0,0,0,0");
}

void Config::setDefault(const QString& key, const char* value)
{
	writeEntry(key,readEntry(key,(const QString)value));
}

void Config::setDefault(const QString& key, const int value)
{
	writeEntry(key,readNumEntry(key,value));
}

void Config::setDefault(const QString& key, const bool value)
{
	writeEntry(key,readBoolEntry(key,value));
}
