// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
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
#include "HelpDialog.hpp"
#include <qpushbutton.h>
#include <qlayout.h>
#include <q3textbrowser.h>
#include <QBoxLayout>

HelpDialog::HelpDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	setCaption(parent->caption());

	QBoxLayout* layout=new QBoxLayout(QBoxLayout::TopToBottom, this);
	Q3TextBrowser* browser=new Q3TextBrowser(this);
	layout->addWidget(browser);
	browser->mimeSourceFactory()
		->addFilePath(Config::instance().
			      readEntry("/hamfax/directories/doc"));
	browser->setSource("hamfax.html");
	QPushButton* button=new QPushButton(tr("&Close"),this);
	layout->addWidget(button);
	connect(button,SIGNAL(clicked()),SLOT(close()));
	resize(300,300);
}

void HelpDialog::close(void)
{
	done(0);
}
