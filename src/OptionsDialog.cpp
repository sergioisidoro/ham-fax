// HamFax -- an application for sending and receiving amateur radio facsimiles
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

#include "OptionsDialog.hpp"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

OptionsDialog::OptionsDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	setCaption(parent->caption());

	QVBoxLayout* layout=new QVBoxLayout(this,15,15);

	QGridLayout* settings=new QGridLayout(layout,3,2);
	settings->addWidget(new QLabel(tr("dsp device"),this),1,1);
	settings->addWidget(devDSP=new QLineEdit(this),1,2);
	settings->addWidget(new QLabel(tr("ptt device"),this),2,1);
	settings->addWidget(devPTT=new QLineEdit(this),2,2);
	settings->addWidget(new QLabel(tr("ptc device"),this),3,1);
	settings->addWidget(devPTC=new QLineEdit(this),3,2);
	settings->addWidget(new QLabel(tr("ptc speed"),this),4,1);
	settings->addWidget(speedPTC=new QComboBox(this),4,2);
	speedPTC->insertItem(tr("38400bps"));
	speedPTC->insertItem(tr("57600bps"));
	speedPTC->insertItem(tr("115200bps"));

	QHBoxLayout* buttons=new QHBoxLayout(layout);
	QPushButton* ok=new QPushButton(tr("&OK"),this);
	QPushButton* cancel=new QPushButton(tr("&Cancel"),this);
	buttons->addWidget(ok);
	buttons->addWidget(cancel);

	connect(ok,SIGNAL(clicked()),this,SLOT(okClicked()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(cancelClicked()));
}

void OptionsDialog::okClicked(void)
{
	done(1);
	emit dsp(devDSP->text());
	emit ptt(devPTT->text());
	emit ptc(devPTC->text());
	switch (speedPTC->currentItem()) {
	case 0: emit ptcSpeed(38400);
		break;
	case 1: emit ptcSpeed(57600);
		break;
	case 2: emit ptcSpeed(115200);
		break;
	};
}

void OptionsDialog::cancelClicked(void)
{
	done(0);
}

void OptionsDialog::setDSP(const QString& s)
{
	devDSP->setText(s);
}

void OptionsDialog::setPTT(const QString& s)
{
	devPTT->setText(s);
}

void OptionsDialog::setPTC(const QString& s)
{
	devPTC->setText(s);
}

void OptionsDialog::setPtcSpeed(int s)
{
	switch(s) {
	case 38400: speedPTC->setCurrentItem(0);
		break;
	case 57600: speedPTC->setCurrentItem(1);
		break;
	case 115200: speedPTC->setCurrentItem(2);
		break;
	};
}

void OptionsDialog::doDialog(void)
{
	exec();
}
