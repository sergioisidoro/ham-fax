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

/**
 * \file
 *
 * HamFax main file. This file implements the main function where
 * everything begins.
 */

#include "config.h"
#include <qapplication.h>
#include <qfont.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qstring.h>
#include "Config.hpp"
#include "FaxWindow.hpp"

/**
 * \mainpage HamFax inline documentation
 *
 * \author Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
 *
 * \section about What is it?
 *
 * The low level documentation is kept (you might have guessed it) as
 * inline comments that can be extracted by doxygen. This approach
 * increases the chances of having current documentation in contrast
 * to the additional work of updating external documents.
 *
 * \todo Split the code in some reusable modules. Namespaces (and
 *       directory names) DSP, QtGUI and Fax?
 */

/**
 * Start here. Only the high level invocations for starting the GUI
 * are here. As much as possible is hidden in appropriate modules and
 * classes.
 *
 * Command line argument are currently used only by Qt.
 *
 * \param argc Argument count.
 * \param argv Argument vector.
 * \return Exit code, also provided by the Qt framework.
 */
 int main(int argc, char* argv[])
{
	const QString version(PACKAGE_STRING);
	Config& config=config.instance();

	QTranslator translator(0);
	translator.load(QString("hamfax_")+QTextCodec::locale(),
			config.readEntry("/hamfax/directories/qm"));

	QApplication app(argc, argv);
	app.installTranslator(&translator);
	FaxWindow* faxWindow=new FaxWindow(version);
	QFont f;
	f.fromString(config.readEntry("/hamfax/GUI/font"));
	app.setFont(f,true);
	app.setMainWidget(faxWindow);
	faxWindow->show();
	return app.exec();
}
