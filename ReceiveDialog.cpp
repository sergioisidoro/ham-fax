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

#include "ReceiveDialog.hpp"
#include <qlayout.h>

ReceiveDialog::ReceiveDialog(QWidget* parent)
	: QDialog(parent)
{
	QGridLayout* layout=new QGridLayout(this,4,1,20,20);
	layout->addWidget(status=new QLabel(this),1,1);
	layout->addWidget(aptText=new QLabel(this),2,1);
	layout->addWidget(skip=new QPushButton(tr("&Skip apt start"),
					       this),3,1);
	connect(skip,SIGNAL(clicked()),this,SIGNAL(skipClicked()));
	layout->addWidget(cancel=new QPushButton(tr("&Cancel"),this),4,1);
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelClicked()));
}

void ReceiveDialog::showText(const QString& s)
{
	status->setText(s);
	adjustSize();
}

void ReceiveDialog::apt(unsigned int f)
{
	aptText->setText(QString(tr("Apt frequency: %1 Hz")).arg(f));
	adjustSize();
}

void ReceiveDialog::closeEvent(QCloseEvent* close)
{
	close->accept();
	emit cancelClicked();
}

void ReceiveDialog::aptStart(void)
{
	showText(tr("searching APT start tone"));
	skip->setDisabled(false);
}

void ReceiveDialog::phasing(void)
{
	skip->setDisabled(true);
	showText(tr("decoding phasing"));
}

void ReceiveDialog::phasingLine(double lpm)
{
	showText(QString(tr("phasing line, lpm %1")).arg(lpm,0,'f',1));
}

void ReceiveDialog::imageRow(unsigned int row)
{
	showText(tr("receiving line %1").arg(row));
}
