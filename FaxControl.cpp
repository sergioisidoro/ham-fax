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

#include "FaxControl.hpp"
#include <qcombobox.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <math.h>

FaxControl::FaxControl(QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout* layout=new QVBoxLayout(this);
	QFrame* mod=new QFrame(this);
	layout->addWidget(mod);
	mod->setFrameStyle(QFrame::Panel|QFrame::Raised);
	QGridLayout* modLayout=new QGridLayout(mod,4,3,10);
	modLayout->addWidget(new QLabel(tr("carrier"),mod),1,1);
	modLayout->addWidget(carrier=new QSpinBox(1500,2400,100,mod),1,2);
	modLayout->addMultiCellWidget(new QLabel(tr("Hz"),mod),1,1,3,4);
	modLayout->addWidget(new QLabel(tr("deviation"),mod),2,1);
	modLayout->addWidget(dev=new QSpinBox(150,500,50,mod),2,2);
	modLayout->addMultiCellWidget(new QLabel(tr("Hz"),mod),2,2,3,4);
	modLayout->addWidget(new QLabel(tr("LPM"),mod),3,1);
	modLayout->addWidget(lpm=new QSpinBox(60,360,10,mod),3,2);
	modLayout->addWidget(new QLabel(tr("modulation"),mod),4,1);
	modLayout->addWidget(modulation=new QComboBox(false,mod),4,2);
	modulation->insertItem(tr("AM"));
	modulation->insertItem(tr("FM"));

	QFrame* a1=new QFrame(this);
	layout->addWidget(a1);
	a1->setFrameStyle(QFrame::Panel|QFrame::Raised);
	QGridLayout* a1Layout=new QGridLayout(a1,2,4,10,10);
	a1Layout->addMultiCellWidget(new QLabel(tr("APT start"),a1),1,1,1,4);
	a1Layout->addWidget(startFreq=new QSpinBox(200,675,10,a1),2,1);
	a1Layout->addWidget(new QLabel(tr("Hz"),a1),2,2);
	a1Layout->addWidget(startLength=new QSpinBox(0,10,1,a1),2,3);
	a1Layout->addWidget(new QLabel(tr("s"),a1),2,4);

	QFrame* a2=new QFrame(this);
	layout->addWidget(a2);
	a2->setFrameStyle(QFrame::Panel|QFrame::Raised);
	QGridLayout* a2Layout=new QGridLayout(a2,2,4,10,10);
	a2Layout->addMultiCellWidget(new QLabel(tr("APT stop"),a2),1,1,1,4);
	a2Layout->addWidget(stopFreq=new QSpinBox(200,675,10,a2),2,1);
	a2Layout->addWidget(new QLabel(tr("Hz"),a2),2,2);
	a2Layout->addWidget(stopLength=new QSpinBox(0,10,1,a2),2,3);
	a2Layout->addWidget(new QLabel(tr("s"),a2),2,4);
	
	QFrame* phs=new QFrame(this);
	layout->addWidget(phs);
	phs->setFrameStyle(QFrame::Panel|QFrame::Raised);
	QGridLayout* phsLayout=new QGridLayout(phs,2,2,10,10);
	phsLayout->addMultiCellWidget(
		new QLabel(tr("phasing lines"),phs),1,1,1,2);
	phsLayout->addWidget(phasing=new QSpinBox(0,20,1,phs),2,1);
	phsLayout->addWidget(phasePol=new QComboBox(false,phs),2,2);
	phasePol->insertItem(tr("normal"));
	phasePol->insertItem(tr("inverted"));

/*	QFrame* size=new QFrame(this);
	layout->addWidget(size);
	size->setFrameStyle(QFrame::Panel|QFrame::Raised);
	QGridLayout* sizeLayout=new QGridLayout(size,3,2,10,10);
	sizeLayout->addWidget(new QLabel(tr("IOC"),size),1,1);
	sizeLayout->addWidget(ioc=new QLabel(size),1,2);
	sizeLayout->addWidget(new QLabel(tr("width"),size),2,1);
	sizeLayout->addWidget(width=new QLabel(size),2,2);
	sizeLayout->addWidget(new QLabel(tr("height"),size),3,1);
	sizeLayout->addWidget(height=new QLabel(size),3,2);
*/	
	QFrame* dummy=new QFrame(this);
	dummy->setFrameStyle(QFrame::Panel|QFrame::Raised);
	layout->addWidget(dummy,1);

	connect(modulation,SIGNAL(activated(int)),
		this,SLOT(newModSelect(int)));
	connect(phasePol,SIGNAL(activated(int)),
		this,SLOT(newPolSelect(int)));
	connect(carrier,SIGNAL(valueChanged(int)),
		this,SLOT(newCarSelect(int)));
	connect(dev,SIGNAL(valueChanged(int)),
		 this,SLOT(newDevSelect(int)));
	connect(lpm,SIGNAL(valueChanged(int)),
		this,SLOT(newLpmSelect(int)));
	connect(startFreq,SIGNAL(valueChanged(int)),
		this,SLOT(newAptStartFreqSel(int)));
	connect(startLength,SIGNAL(valueChanged(int)),
		this,SLOT(newAptStartLengSel(int)));
	connect(stopFreq,SIGNAL(valueChanged(int)),
		this,SLOT(newAptStopFreqSel(int)));
	connect(stopLength,SIGNAL(valueChanged(int)),
		this,SLOT(newAptStopLengSel(int)));
	connect(phasing,SIGNAL(valueChanged(int)),
		this,SLOT(newPhaseLinesSel(int)));
}

void FaxControl::newModSelect(int index)
{
	emit newModulation(index==1 ? true : false);
}

void FaxControl::setModulation(bool fm)
{
	modulation->setCurrentItem(fm ? 1 : 0);
}

void FaxControl::setCarrier(unsigned int carrier)
{
	this->carrier->setValue(carrier);
}

void FaxControl::setDeviation(unsigned int deviation)
{
	dev->setValue(deviation);
}

void FaxControl::setLPM(unsigned int lpm)
{
	this->lpm->setValue(lpm);
}

void FaxControl::setAptStartFreq(unsigned int f)
{
	startFreq->setValue(f);
}

void FaxControl::setAptStartLength(unsigned int t)
{
	startLength->setValue(t);
}

void FaxControl::setAptStopFreq(unsigned int f)
{
	stopFreq->setValue(f);
}

void FaxControl::setAptStopLength(unsigned int t)
{
	stopLength->setValue(t);
}

void FaxControl::setPhasingLines(unsigned int lines)
{
	phasing->setValue(lines);
}

void FaxControl::newPolSelect(int index)
{
	emit newPhasingPol(index==0 ? true : false);
}

void FaxControl::setPhasingPol(bool pol)
{
	phasePol->setCurrentItem(pol ? 0 : 1);
}

void FaxControl::newCarSelect(int carrier)
{
	emit newCarrier((unsigned int)carrier);
}

void FaxControl::newDevSelect(int dev)
{
	emit newDeviation((unsigned int)dev);
}

void FaxControl::newLpmSelect(int lpm)
{
	emit newLPM((unsigned int)lpm);
}

void FaxControl::newAptStartFreqSel(int f)
{
	emit newAptStartFreq((unsigned int)f);
}

void FaxControl::newAptStartLengSel(int l)
{
	emit newAptStartLength((unsigned int)l);
}

void FaxControl::newAptStopFreqSel(int f)
{
	emit newAptStopFreq((unsigned int)f);
}

void FaxControl::newAptStopLengSel(int l)
{
	emit newAptStopLength((unsigned int)l);
}

void FaxControl::newPhaseLinesSel(int n)
{
	emit newPhasingLength((unsigned int)n);
}
