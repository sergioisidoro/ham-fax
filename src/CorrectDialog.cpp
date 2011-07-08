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

#include "CorrectDialog.hpp"
#include <qpushbutton.h>
#include <qlayout.h>

CorrectDialog::CorrectDialog(QWidget* parent)
	: QDialog(parent)
{
	setCaption(parent->caption());
	QVBoxLayout* layout=new QVBoxLayout(this,15,15);
	layout->addWidget(text=new QLabel(this));
	QPushButton* cancel=new QPushButton(tr("&Cancel"),this);
	layout->addWidget(cancel);
	connect(cancel,SIGNAL(clicked()),SIGNAL(cancelClicked()));
	connect(cancel,SIGNAL(clicked()),SLOT(hideWindow()));
}

void CorrectDialog::setText(const QString& s)
{
	text->setText(s);
}

void CorrectDialog::reject(void)
{
	// the escape key leads here
	emit cancelClicked();
	hide();
}

void CorrectDialog::hideWindow(void)
{
	hide();
}
