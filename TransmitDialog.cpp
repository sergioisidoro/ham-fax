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

#include "TransmitDialog.hpp"
#include <qlayout.h>

TransmitDialog::TransmitDialog(QWidget* parent)
	: QDialog(parent)
{
	QVBoxLayout* layout=new QVBoxLayout(this);
	layout->setSpacing(20);
	layout->setMargin(20);
	status=new QLabel(this);
	layout->addWidget(status);
	cancel=new QPushButton(tr("&Cancel"),this);
	layout->addWidget(cancel);
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelClicked()));
}

void TransmitDialog::showText(const QString& s)
{
	status->setText(s);
	adjustSize();
}

void TransmitDialog::closeEvent(QCloseEvent* close)
{
	close->accept();
	emit cancelClicked();
}
