// hamfax -- an application for sending and receiving amateur radio facsimiles
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

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <memory>
#include <qsettings.h>
#include <qstring.h>

/**
 * This class is implemented as a Singleton and provides a global interface to 
 * configuration data. It is derived from QSettings, so see the Qt documentation
 * for specific member functions.
 * The constructor initializes everything with default values in case there is 
 * no item found in the configuration file.
 */

class Config : public QSettings {
public:
        /**
	 * Only possible access to Singleton
	 */
	static Config& instance();

	QString readEntry(const QString& key);
	bool readBoolEntry(const QString &key);
	int readNumEntry(const QString& key);
	void writeEntry(const QString& key, const QString& value);
	void writeEntry(const QString& key, bool value);
	void writeEntry(const QString& key, int value);
private:
	typedef std::auto_ptr<Config> ConfigPtr;
	friend class std::auto_ptr<Config>;
	Config();
	~Config() {};
	void setDefault(const QString& key, const char* value);
	void setDefault(const QString& key, const int value);
	void setDefault(const QString& key, const bool value);
};

#endif
