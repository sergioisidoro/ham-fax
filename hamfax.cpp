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

#include <qapplication.h>
#include <qtranslator.h>
#include <qstring.h>
#include <locale.h>
#include "FaxWindow.hpp"

int main(int argc, char* argv[])
{
	const QString version="0.0";

	QApplication app(argc, argv);
	QTranslator translator(0);
	translator.load(QString("hamfax.")+setlocale(LC_MESSAGES,0),".");
	FaxWindow* faxWindow=new FaxWindow(version);
	app.installTranslator(&translator);
	app.setMainWidget(faxWindow);
	faxWindow->show();
	return app.exec();
}
