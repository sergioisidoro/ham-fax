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

#include "OptionsDialog.hpp"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	QVBoxLayout* layout=new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setMargin(10);

	QGridLayout* settings=new QGridLayout(layout,3,2);
	settings->addWidget(new QLabel(tr("dsp device"),this),1,1);
	devDSP=new QLineEdit(this);
	settings->addWidget(devDSP,1,2);
	settings->addWidget(new QLabel(tr("ptt device"),this),2,1);
	devPTT=new QLineEdit(this);
	settings->addWidget(devPTT,2,2);
	settings->addWidget(new QLabel(tr("ptc device"),this),3,1);
	devPTC=new QLineEdit(this);
	settings->addWidget(devPTC,3,2);

	QHBoxLayout* buttons=new QHBoxLayout(layout);
	QPushButton* ok=new QPushButton(tr("&OK"),this);
	QPushButton* cancel=new QPushButton(tr("&Cancel"),this);
	buttons->addWidget(ok);
	buttons->addWidget(cancel);

	connect(devDSP,SIGNAL(textChanged(const QString&)),
		this,SLOT(dspChanged(const QString&)));
	connect(devPTT,SIGNAL(textChanged(const QString&)),
		this,SLOT(pttChanged(const QString&)));
	connect(devPTC,SIGNAL(textChanged(const QString&)),
		this,SLOT(ptcChanged(const QString&)));
	connect(ok,SIGNAL(clicked()),this,SLOT(okClicked()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(cancelClicked()));
}

void OptionsDialog::init(void)
{
	devDSP->setText(devDSPName);
	devPTT->setText(devPTTName);
	devPTC->setText(devPTCName);
}

void OptionsDialog::okClicked(void)
{
	done(1);
}

void OptionsDialog::cancelClicked(void)
{
	done(0);
}

void OptionsDialog::dspChanged(const QString& s)
{
	devDSPName=s;
}

void OptionsDialog::pttChanged(const QString& s)
{
	devPTTName=s;
}

void OptionsDialog::ptcChanged(const QString& s)
{
	devPTCName=s;
}
