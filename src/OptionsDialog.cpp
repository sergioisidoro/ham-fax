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

#include "OptionsDialog.hpp"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "Config.hpp"

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	Config& config=Config::instance();
	setCaption(parent->caption());
	QVBoxLayout* layout=new QVBoxLayout(this,15,15);

	QGridLayout* settings=new QGridLayout(layout,3,2);
	settings->addWidget(new QLabel(tr("dsp device"),this),1,1);
	settings->addWidget(devDSP=new QLineEdit(this),1,2);
	devDSP->setText(config.getDSP());

	settings->addWidget(new QLabel(tr("ptt device"),this),2,1);
	settings->addWidget(devPTT=new QLineEdit(this),2,2);
	devPTT->setText(config.getPTT());

	settings->addWidget(new QLabel(tr("ptc device"),this),3,1);
	settings->addWidget(devPTC=new QLineEdit(this),3,2);
	devPTC->setText(config.getPTC());

	settings->addWidget(new QLabel(tr("ptc speed"),this),4,1);
	settings->addWidget(speedPTC=new QComboBox(this),4,2);
	speedPTC->insertItem(tr("38400bps"));
	speedPTC->insertItem(tr("57600bps"));
	speedPTC->insertItem(tr("115200bps"));
	int i=0;
	switch(config.getPtcSpeed()) {
	case 115200: i=2; break;
	case 57600:  i=1; break;
	default:          break;
	};
	speedPTC->setCurrentItem(i);

	QHBoxLayout* buttons=new QHBoxLayout(layout);
	QPushButton* ok=new QPushButton(tr("&OK"),this);
	buttons->addWidget(ok);
	connect(ok,SIGNAL(clicked()),this,SLOT(okClicked()));

	QPushButton* cancel=new QPushButton(tr("&Cancel"),this);
	buttons->addWidget(cancel);
	connect(cancel,SIGNAL(clicked()),this,SLOT(cancelClicked()));
}

void OptionsDialog::okClicked(void)
{
	Config& config=Config::instance();
	done(1);

	int s=38400;
	switch (speedPTC->currentItem()) {
	case 2: s=115200; break;
	case 1: s=57600;  break;
	default:          break;
	};
	config.setPtcSpeed(s);
	config.setDSP(devDSP->text());
	config.setPTT(devPTT->text());
	config.setPTC(devPTC->text());
}

void OptionsDialog::cancelClicked(void)
{
	done(0);
}
