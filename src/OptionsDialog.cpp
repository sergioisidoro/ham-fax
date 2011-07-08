// hamfax -- an application for sending and receiving amateur radio facsimiles
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

#include "Config.hpp"
#include "OptionsDialog.hpp"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include "config.h"

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	Config& c=Config::instance();
	setCaption(parent->caption());
	QVBoxLayout* layout=new QVBoxLayout(this,15,15);

	QGridLayout* settings=new QGridLayout(layout,3,2);
	settings->addWidget(new QLabel(tr("dsp device"),this),1,1);
	settings->addWidget(devDSP=new QLineEdit(this),1,2);
	devDSP->setText(c.readEntry("/hamfax/sound/device"));

	settings->addWidget(new QLabel(tr("ptt device"),this),2,1);
	settings->addWidget(devPTT=new QLineEdit(this),2,2);
	devPTT->setText(c.readEntry("/hamfax/PTT/device"));

	settings->addWidget(new QLabel(tr("ptc device"),this),3,1);
	settings->addWidget(devPTC=new QLineEdit(this),3,2);
	devPTC->setText(c.readEntry("/hamfax/PTC/device"));

	settings->addWidget(new QLabel(tr("ptc speed"),this),4,1);
	settings->addWidget(speedPTC=new QComboBox(this),4,2);
	speedPTC->insertItem(tr("38400bps"));
	speedPTC->insertItem(tr("57600bps"));
	speedPTC->insertItem(tr("115200bps"));
	int i=0;
	switch(c.readNumEntry("/hamfax/PTC/speed")) {
	case 115200: i=2; break;
	case 57600:  i=1; break;
	default:          break;
	};
	speedPTC->setCurrentItem(i);

#ifdef HAVE_LIBHAMLIB
	settings->addWidget(new QLabel(tr("hamlib model number"),this),5,1);
	settings->addWidget(hamlibModel=new QLineEdit(this),5,2);
	hamlibModel->setText(c.readEntry("/hamfax/HAMLIB/hamlib_model"));

	settings->addWidget(new QLabel(tr("hamlib optional parameters"),this),6,1);
	settings->addWidget(hamlibParams=new QLineEdit(this),6,2);
	hamlibParams->setText(c.readEntry("/hamfax/HAMLIB/hamlib_parameters"));
#endif

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
	done(1);
	Config& c=Config::instance();
	int s=38400;
	switch (speedPTC->currentItem()) {
	case 2: s=115200; break;
	case 1: s=57600;  break;
	default:          break;
	};
	c.writeEntry("/hamfax/PTC/speed",s);
	c.writeEntry("/hamfax/PTC/device",devPTC->text());
	c.writeEntry("/hamfax/sound/device",devDSP->text());
	c.writeEntry("/hamfax/PTT/device",devPTT->text());
#ifdef HAVE_LIBHAMLIB
	c.writeEntry("/hamfax/HAMLIB/hamlib_model",hamlibModel->text());
	c.writeEntry("/hamfax/HAMLIB/hamlib_parameters",hamlibParams->text());
#endif
}

void OptionsDialog::cancelClicked(void)
{
	done(0);
}
