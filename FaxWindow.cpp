// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt <cschmit@suse.de>
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

#include "FaxWindow.hpp"
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include "Error.hpp"
#include "FaxView.hpp"
#include "OptionsDialog.hpp"
#include "ScaleDialog.hpp"

FaxWindow::FaxWindow(const QString& version)
	: version(version)
{
	faxImage=new FaxImage(this);
	FaxView* faxView=new FaxView(this,faxImage);
	faxControl=new FaxControl(this);

	faxTransmitter=new FaxTransmitter(this,faxImage);
	faxReceiver=new FaxReceiver(this,faxImage);
	
	sound=new Sound(this);
	file=new File(this);
	ptt=new PTT(this);
	ptc=new PTC(this);
	faxModulator=new FaxModulator(this);
	faxDemodulator=new FaxDemodulator(this);
	timer=new QTimer(this);
	transmitDialog=new TransmitDialog(this);
	receiveDialog=new ReceiveDialog(this);

	buildMenuBar();

	QGridLayout* layout=new QGridLayout(this,2,2);
	layout->setRowStretch(1,0);
	layout->setRowStretch(2,1);
	layout->setColStretch(1,0);
	layout->setColStretch(2,1);
	layout->setMenuBar(menuBar);
	layout->addWidget(faxControl,1,1);
	layout->addMultiCellWidget(faxView,1,2,2,2);

	// FaxControl -- FaxTransmitter -- FaxReceiver
	connect(faxControl,SIGNAL(newLPM(int)),
		faxTransmitter,SLOT(setLPM(int)));
	connect(faxTransmitter,SIGNAL(newLPM(int)),
		faxControl,SLOT(setLPM(int)));
	connect(faxControl,SIGNAL(newAptStartFreq(int)),
		faxTransmitter,SLOT(setAptStartFreq(int)));
	connect(faxTransmitter,SIGNAL(newAptStartFreq(int)),
		faxControl,SLOT(setAptStartFreq(int)));
	connect(faxTransmitter,SIGNAL(newAptStartFreq(int)),
		faxReceiver,SLOT(setAptStartFreq(int)));
	connect(faxControl,SIGNAL(newAptStartLength(int)),
		faxTransmitter,SLOT(setAptStartLength(int)));
	connect(faxTransmitter,SIGNAL(newAptStartLength(int)),
		faxControl,SLOT(setAptStartLength(int)));
	connect(faxControl,SIGNAL(newAptStopFreq(int)),
		faxTransmitter,SLOT(setAptStopFreq(int)));
	connect(faxTransmitter,SIGNAL(newAptStopFreq(int)),
		faxControl,SLOT(setAptStopFreq(int)));
	connect(faxTransmitter,SIGNAL(newAptStopFreq(int)),
		faxReceiver,SLOT(setAptStopFreq(int)));
	connect(faxControl,SIGNAL(newAptStopLength(int)),
		faxTransmitter,SLOT(setAptStopLength(int)));
	connect(faxTransmitter,SIGNAL(newAptStopLength(int)),
		faxControl,SLOT(setAptStopLength(int)));
	connect(faxControl,SIGNAL(newPhasingLength(int)),
		faxTransmitter,SLOT(setPhasingLines(int)));
	connect(faxTransmitter,SIGNAL(newPhasingLines(int)),
		faxControl,SLOT(setPhasingLines(int)));
	connect(faxControl,SIGNAL(newPhasingPol(bool)),
		faxTransmitter,SLOT(setPhasePol(bool)));
	connect(faxTransmitter,SIGNAL(newPhasePol(bool)),
		faxControl,SLOT(setPhasingPol(bool)));
	connect(faxTransmitter,SIGNAL(newPhasePol(bool)),
		faxReceiver,SLOT(setPhasePol(bool)));
	faxTransmitter->setLPM(120);
	faxTransmitter->setAptStartFreq(300);
	faxTransmitter->setAptStartLength(5);
	faxTransmitter->setAptStopFreq(450);
	faxTransmitter->setAptStopLength(5);
	faxTransmitter->setPhasingLines(20);
	faxTransmitter->setPhasePol(true);

	// FaxControl -- FaxModulator -- FaxDemodulator
	connect(faxControl,SIGNAL(newCarrier(int)),
		faxModulator,SLOT(setCarrier(int)));
	connect(faxModulator,SIGNAL(newCarrier(int)),
		faxControl,SLOT(setCarrier(int)));
	connect(faxModulator,SIGNAL(newCarrier(int)),
		faxDemodulator,SLOT(setCarrier(int)));
	connect(faxControl,SIGNAL(newDeviation(int)),
		faxModulator,SLOT(setDeviation(int)));
	connect(faxModulator,SIGNAL(newDeviation(int)),
		faxControl,SLOT(setDeviation(int)));
	connect(faxModulator,SIGNAL(newDeviation(int)),
		faxDemodulator,SLOT(setDeviation(int)));
	connect(faxControl,SIGNAL(newModulation(bool)),
		faxModulator,SLOT(setFM(bool)));
	connect(faxModulator,SIGNAL(newModulation(bool)),
		faxControl,SLOT(setModulation(bool)));
	connect(faxModulator,SIGNAL(newModulation(bool)),
		faxDemodulator,SLOT(setFM(bool)));
	connect(faxModulator,SIGNAL(newModulation(bool)),
		ptc,SLOT(setFM(bool)));
	faxModulator->setFM(true);
	faxModulator->setCarrier(1900);
	faxModulator->setDeviation(400);

	ptt->setUse(false);
	ptt->setDeviceName("/dev/ttyS1");

	// FaxWindow -- FaxImage -- FaxView
	connect(this,SIGNAL(loadFile(QString)),faxImage,SLOT(load(QString)));
	connect(this,SIGNAL(saveFile(QString)),faxImage,SLOT(save(QString)));
	connect(faxImage,SIGNAL(sizeUpdated(unsigned int, unsigned int)),
		faxView,SLOT(updateView(unsigned int, unsigned int)));
	connect(faxImage,SIGNAL(sizeUpdated(unsigned int, unsigned int)),
		faxControl,SLOT(setImageSize(unsigned int, unsigned int)));
	connect(faxImage,SIGNAL(widthUpdated(unsigned int)),
		faxReceiver,SLOT(setWidth(unsigned int)));
	connect(faxImage,SIGNAL(contentUpdated(int,int,int,int)),
		faxView,SLOT(update(int,int,int,int)));
	faxImage->create(904,904);

	connect(faxTransmitter,SIGNAL(statusText(const QString&)),
		transmitDialog,SLOT(showText(const QString&)));
	connect(faxReceiver,SIGNAL(statusText(const QString&)),
		receiveDialog,SLOT(showText(const QString&)));
	connect(faxReceiver,SIGNAL(aptText(const QString&)),
		receiveDialog,SLOT(showApt(const QString&)));
	connect(faxReceiver,SIGNAL(aptStopDetected()),
		this,SLOT(endReception()));
	connect(receiveDialog,SIGNAL(skipClicked()),
		faxReceiver,SLOT(skipAptStart()));

	connect(receiveDialog,SIGNAL(cancelClicked()),
		this,SLOT(endReception()));
	connect(transmitDialog,SIGNAL(cancelClicked()),
		this,SLOT(endTransmission()));

	ptc->setDeviceName("/dev/ttyS0");
	sound->setDSPDevice("/dev/dsp");
}

void FaxWindow::buildMenuBar(void)
{
	QPopupMenu* fileMenu=new QPopupMenu(this);
	fileMenu->insertItem(tr("&Open"),this,SLOT(load()));
	fileMenu->insertItem(tr("&Save"),this,SLOT(save()));
	fileMenu->insertSeparator();
	fileMenu->insertItem(tr("&Exit"),this,SLOT(close()));

	QPopupMenu* transmitMenu=new QPopupMenu(this);
	transmitMenu->insertItem(tr("Transmit using &dsp"),DSP);
	transmitMenu->insertItem(tr("Transmit to &file"),FILE);
	transmitMenu->insertItem(tr("Transmit using &PTC"),SCSPTC);

	QPopupMenu* receiveMenu=new QPopupMenu(this);
	receiveMenu->insertItem(tr("Receive from d&sp"),DSP);
	receiveMenu->insertItem(tr("Receive from f&ile"),FILE);
	receiveMenu->insertItem(tr("Receive from P&TC"),SCSPTC);

	optionsMenu=new QPopupMenu(this);
	optionsMenu->insertItem(tr("device settings"),
				this,SLOT(doOptionsDialog()));
	optionsMenu->insertItem(tr("&Scale image / adjust IOC"),
			    this,SLOT(doScaleDialog()));
	optionsMenu->insertSeparator();
	pttID=optionsMenu->
		insertItem(tr("key PTT while transmitting with DSP"),
			   this,SLOT(changePTT()));
	optionsMenu->setItemChecked(pttID,false);

	QPopupMenu* helpMenu=new QPopupMenu(this);
	helpMenu->insertItem(tr("&About"),this,SLOT(about()));
	helpMenu->insertItem(tr("About &QT"),this,SLOT(aboutQT()));

	menuBar=new QMenuBar(this);
	menuBar->insertItem(tr("&File"),fileMenu);
	menuBar->insertItem(tr("&Transmit"),transmitMenu);
	menuBar->insertItem(tr("&Receive"),receiveMenu);
	menuBar->insertItem(tr("&Options"),optionsMenu);
	menuBar->insertSeparator();
	menuBar->insertItem(tr("&Help"),helpMenu);

	connect(transmitMenu,SIGNAL(activated(int)),
		this,SLOT(initTransmit(int)));
	connect(receiveMenu,SIGNAL(activated(int)),
		this,SLOT(initReception(int)));
}

void FaxWindow::about(void)
{
	QMessageBox::information(this,this->caption(),
				 tr("HamFax is a QT application for"
				    "transmitting and receiving \n"
				    "ham radio facsimiles\nVersion %1")
		.arg(version));
}

void FaxWindow::aboutQT(void)
{
	QMessageBox::aboutQt(this,this->caption());
}

void FaxWindow::load(void)
{
	QFileDialog* fd=new QFileDialog(this,0,true);
	fd->setSizeGripEnabled(false);
	fd->setCaption(tr("open file"));
	fd->setFilter("*.png");
	fd->exec();
	if(!fd->selectedFile().isEmpty()) {
		emit loadFile(fd->selectedFile());
	}
}

void FaxWindow::save(void)
{
	QFileDialog* fd=new QFileDialog(this,0,true);
	fd->setSizeGripEnabled(false);
	fd->setCaption(tr("save file"));
	fd->setFilter("*.png");
	fd->exec();
	if(!fd->selectedFile().isEmpty()) {
		emit saveFile(fd->selectedFile());
	}
}

void FaxWindow::doOptionsDialog(void)
{
	OptionsDialog* d=new OptionsDialog(this);
	d->devDSPName=sound->getDSPDevice();
	d->devPTTName=ptt->getDeviceName();
	d->devPTCName=ptc->getDeviceName();
	d->init();
	if(d->exec()) {
		sound->setDSPDevice(d->devDSPName);
		ptt->setDeviceName(d->devPTTName);
		ptc->setDeviceName(d->devPTCName);
	}
	delete d;
}

void FaxWindow::doScaleDialog(void)
{
	ScaleDialog* d=new ScaleDialog(this);
	d->width=faxImage->getCols();
	d->height=faxImage->getRows();
	d->init();
	if(d->exec()) {
		faxImage->scale(d->width,d->height);
	}
	delete d;
}

void FaxWindow::changePTT(void)
{
	if(optionsMenu->isItemChecked(pttID)) {
		optionsMenu->setItemChecked(pttID,false);
		ptt->setUse(false);
	} else {
		optionsMenu->setItemChecked(pttID,true);
		ptt->setUse(true);
	}
}

QString FaxWindow::getFileName(QString caption, QString filter)
{
	QFileDialog* fd=new QFileDialog(this,0,true);
	fd->setSizeGripEnabled(false);
	fd->setCaption(caption);
	fd->setFilter(filter);
	fd->exec();
	QString s=fd->selectedFile();
	delete fd;
	return s;
}

void FaxWindow::initTransmit(int item)
{
	try {
		unsigned int sampleRate=0;
		QString fileName;
		switch(interface=item) {
		case FILE:
			sampleRate=8000;
			fileName=getFileName(tr("output file name"),"*.au");
			if(fileName.isEmpty()) {
				return;
			}
			file->openOutput(fileName,sampleRate);
			break;
		case DSP:
			sampleRate=8000;
			sound->openOutput(sampleRate);
			ptt->openDevice();
			ptt->set(true);
			break;
		case SCSPTC:
			sampleRate=5760;
			ptc->open();
			break;
		}
		faxModulator->setSampleRate(sampleRate);
		faxTransmitter->setSampleRate(sampleRate);
		faxTransmitter->startTransmission();
		menuBar->setDisabled(true);
		faxControl->setDisabled(true);
		timer->start(0);
		connect(timer,SIGNAL(timeout()),this,SLOT(transmitNext()));
		transmitDialog->show();

	} catch (Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::transmitNext(void)
{
	try {
		unsigned int n=512;
		signed short sample[n];
		double buffer[n];
		faxTransmitter->getValues(buffer,n);
		if(n>0) {
			switch(interface) {
			case SCSPTC:
				ptc->transmit(buffer,n);
				break;
			case DSP:
				faxModulator->modulate(sample,buffer,n);
				sound->write(sample,n);
				break;
			case FILE:
				faxModulator->modulate(sample,buffer,n);
				file->write(sample,n);
				break;
			}
		} else {
			if(interface!=DSP || sound->outputBufferEmpty()) {
				endTransmission();
			}
		}
	} catch(Error e) {
		endTransmission();
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::endTransmission(void)
{
	timer->stop();
	disconnect(timer,SIGNAL(timeout()),this,SLOT(transmitNext()));
	switch(interface) {
	case FILE:
		file->close();
		break;
	case DSP:
		ptt->set(false);
		sound->close();
		ptt->closeDevice();
		break;
	case SCSPTC:
		ptc->close();
	}
	menuBar->setDisabled(false);
	faxControl->setDisabled(false);
	transmitDialog->hide();
}

void FaxWindow::initReception(int item)
{
	try {
		unsigned int sampleRate;
		QString fileName;
		switch(interface=item) {
		case FILE:
			fileName=getFileName(tr("input file name"),"*.au");
			if(fileName.isEmpty()) {
				return;
			}
			file->openInput(fileName,sampleRate);
			break;
		case DSP:
			sampleRate=8000;
			sound->openInput(sampleRate);
			break;
		case SCSPTC:
			sampleRate=5760;
			ptc->open();
			break;
		}
		faxDemodulator->setSampleRate(sampleRate);
		faxReceiver->setSampleRate(sampleRate);
		faxReceiver->init();
		menuBar->setDisabled(true);
		faxControl->setDisabled(true);
		timer->start(0);
		connect(timer,SIGNAL(timeout()),
			this,SLOT(receiveNext()));
		receiveDialog->show();
	} catch(Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::receiveNext(void)
{
	unsigned int n=256;
	signed short samples[n];
	unsigned int buffer[n];
	switch(interface) {
	case FILE:
		file->read(samples,n);
		faxDemodulator->demodulate(buffer,samples,n);
		if(n==0) {
			endReception();
		}
		break;
	case DSP:
		sound->read(samples,n);
		faxDemodulator->demodulate(buffer,samples,n);
		break;
	case SCSPTC:
		ptc->receive(buffer,n);
		break;
	}
	faxReceiver->decode(buffer,n);
}

void FaxWindow::endReception(void)
{
	timer->stop();
	disconnect(timer,SIGNAL(timeout()),this,SLOT(receiveNext()));
	switch(interface) {
	case FILE:
		file->close();
		break;
	case DSP:
		sound->close();
		break;
	case SCSPTC:
		ptc->close();
		break;
	}
	menuBar->setDisabled(false);
	faxControl->setDisabled(false);
	receiveDialog->hide();
}

void FaxWindow::closeEvent(QCloseEvent* close)
{
	switch(QMessageBox::information(this,this->caption(),
					tr("Really exit?"),
					tr("&Exit"),tr("&Don't Exit"))) {
	case 0:
		timer->stop();
		close->accept();
		break;
	case 1:
		break;
	}
}
