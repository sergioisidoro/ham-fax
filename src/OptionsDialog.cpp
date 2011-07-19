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

#include <QLabel>
#include <QPushButton>
#include "Config.hpp"
#include "OptionsDialog.hpp"

QLineEdit* OptionsDialog::addItem(const QString& name, const QString& config)
{

	QLineEdit *lineEdit = new QLineEdit();
	lineEdit->setText(Config::instance().readEntry("/hamfax/" + config));

	settings->addWidget(new QLabel(name), row, 1);
	settings->addWidget(lineEdit, row, 2);

	row++;
	return lineEdit;
}

OptionsDialog::OptionsDialog(QWidget* parent)
                : row(1)
{
	Config& c=Config::instance();
	setWindowTitle(parent->windowTitle());
	QBoxLayout* layout=new QBoxLayout(QBoxLayout::TopToBottom, this);

	settings = new QGridLayout();
	layout->addLayout(settings);

	devDSP = addItem(tr("dsp device"), "sound/device");
	devPTT = addItem(tr("ptt device"), "PTT/device");
	devPTC = addItem(tr("ptc device"), "PTC/device");

	settings->addWidget(new QLabel(tr("ptc speed"), this),row , 1);
	settings->addWidget(speedPTC = new QComboBox(this), row++, 2);
	speedPTC->addItem(tr("38400bps"));
	speedPTC->addItem(tr("57600bps"));
	speedPTC->addItem(tr("115200bps"));
	int i=0;
	switch(c.readNumEntry("/hamfax/PTC/speed")) {
	case 115200: i=2; break;
	case 57600:  i=1; break;
	default:          break;
	};
	speedPTC->setCurrentIndex(i);

#ifdef HAVE_LIBHAMLIB
	hamlibModel = addItem(tr("hamlib model number"), "HAMLIB/hamlib_model");
	hamlibParams = addItem(tr("hamlib optional parameters"),
			       "HAMLIB/hamlib_parameters");
#endif

	QBoxLayout* buttons=new QBoxLayout(QBoxLayout::LeftToRight);
	layout->addLayout(buttons);
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
	switch (speedPTC->currentIndex()) {
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
