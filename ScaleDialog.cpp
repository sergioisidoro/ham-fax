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

#include "ScaleDialog.hpp"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <math.h>

ScaleDialog::ScaleDialog(QWidget* parent)
	: QDialog(parent,0,true)
{
	QVBoxLayout* layout=new QVBoxLayout(this,15,15);

	QGridLayout* settings=new QGridLayout(layout,3,2);
	settings->addWidget(new QLabel(tr("IOC:"),this),1,1);
	ioc=new QSpinBox(204,576,1,this);

	settings->addWidget(ioc,1,2);
	settings->addWidget(new QLabel(tr("width:"),this),2,1);
	widthLabel=new QLabel(this);
	settings->addWidget(widthLabel,2,2);
	settings->addWidget(new QLabel(tr("height:"),this),3,1);
	heightLabel=new QLabel(this);
	settings->addWidget(heightLabel,3,2);

	QHBoxLayout* buttons=new QHBoxLayout(layout);
	QPushButton* ok=new QPushButton(tr("OK"),this);
	QPushButton* cancel=new QPushButton(tr("Cancel"),this);
	buttons->addWidget(ok);
	buttons->addWidget(cancel);

	connect(ioc,SIGNAL(valueChanged(int)),
		this,SLOT(newIOC(int)));
	connect(ok,SIGNAL(clicked()),this,SLOT(okClicked()));
	connect(cancel,SIGNAL(clicked()),this,SLOT(cancelClicked()));
}

void ScaleDialog::newIOC(int ioc)
{
	unsigned int newWidth=(unsigned int)((double)ioc*M_PI);
	unsigned int newHeight=(unsigned int)
		(fabs((double)newWidth*(double)height/(double)width+0.5));
	widthLabel->setText(QString().setNum(newWidth));
	heightLabel->setText(QString().setNum(newHeight));
}

void ScaleDialog::okClicked(void)
{
	width=widthLabel->text().toUInt();
	height=heightLabel->text().toUInt();
	emit imageWidth(width);
	done(1);
}

void ScaleDialog::cancelClicked(void)
{
	done(0);
}

void ScaleDialog::setSize(int w, int h)
{
	width=w;
	height=h;
}

void ScaleDialog::doDialog(void)
{
	ioc->setValue((unsigned int)(fabs((double)width/M_PI+0.5)));
	newIOC(ioc->value());
	exec();
}
