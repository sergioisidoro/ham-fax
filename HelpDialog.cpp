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

#include "HelpDialog.hpp"
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtextbrowser.h>

#ifndef DOCDIR
#error "DOCDIR needs to be defined"
#endif

HelpDialog::HelpDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	setCaption(parent->caption());

	QVBoxLayout* layout=new QVBoxLayout(this,15,15);
	QTextBrowser* browser=new QTextBrowser(this);
	layout->addWidget(browser);
	browser->mimeSourceFactory()->addFilePath(DOCDIR);

	browser->setSource("HamFax.html");
	QPushButton* button=new QPushButton(tr("&Close"),this);
	layout->addWidget(button);
	connect(button,SIGNAL(clicked()),SLOT(close()));
	resize(300,300);
}

void HelpDialog::close(void)
{
	done(0);
}
