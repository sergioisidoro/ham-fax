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

#include "ReceiveDialog.hpp"
#include <qlayout.h>

ReceiveDialog::ReceiveDialog(QWidget* parent)
	: QDialog(parent)
{
        setCaption(parent->caption());
	QVBoxLayout* layout=new QVBoxLayout(this,15,15);
	layout->addWidget(status=new QLabel(this));
	status->setMinimumWidth(200);
	layout->addWidget(aptText=new QLabel(this));
	layout->addWidget(level=new DisplayLevel(this));
	layout->addWidget(spectrum=new Spectrum(this));
	layout->addWidget(skip=new QPushButton(this));
	connect(skip,SIGNAL(clicked()),this,SIGNAL(skipClicked()));
	layout->addWidget(cancel=new QPushButton(tr("&Cancel"),this));
	connect(cancel,SIGNAL(clicked()),this,SIGNAL(cancelClicked()));
}

void ReceiveDialog::apt(int f)
{
	aptText->setText(QString(tr("Apt frequency: %1 Hz")).arg(f));
}

void ReceiveDialog::closeEvent(QCloseEvent* close)
{
	close->ignore();
	if(cancel->isEnabled()) {
		emit cancelClicked();
	}
}

void ReceiveDialog::aptStart(void)
{
	status->setText(tr("searching APT start tone"));
	skip->setDisabled(false);
	level->setZero();
	skip->setText(tr("&Skip apt start"));
	show();
}

void ReceiveDialog::phasing(void)
{
	status->setText(tr("decoding phasing"));
	skip->setText(tr("skip phasing"));
	skip->setDisabled(false);
}

void ReceiveDialog::phasingLine(double lpm)
{
	status->setText(QString(tr("phasing line, lpm %1")).arg(lpm,0,'f',1));
	skip->setDisabled(true);
}

void ReceiveDialog::imageRow(int row)
{
	status->setText(tr("receiving line %1").arg(row));
}

void ReceiveDialog::samples(short* buffer, int n)
{
	level->samples(buffer,n);
}

void ReceiveDialog::imageData(int* buffer, int n)
{
	spectrum->samples(buffer,n);
}

void ReceiveDialog::disableSkip(void)
{
	skip->setDisabled(true);
}
