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

#include "FaxWindow.hpp"
#include <qpopupmenu.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <math.h>
#include "Error.hpp"
#include "HelpDialog.hpp"
#include "OptionsDialog.hpp"
#include "ScaleDialog.hpp"
#include "ReceiveDialog.hpp"
#include "PTT.hpp"

FaxWindow::FaxWindow(const QString& version)
	: version(version)
{
	config=new Config(this);
	setCentralWidget(faxImage=new FaxImage(this));
	faxReceiver=new FaxReceiver(this);
	faxTransmitter=new FaxTransmitter(this,faxImage);
	file=new File(this);
	ptc=new PTC(this);
	sound=new Sound(this);
	PTT* ptt=new PTT(this);
	faxModulator=new FaxModulator(this);
	faxDemodulator=new FaxDemodulator(this);
	transmitDialog=new TransmitDialog(this);
	ReceiveDialog* receiveDialog=new ReceiveDialog(this);
	correctDialog=new CorrectDialog(this);

	statusBar()->setSizeGripEnabled(false);
	statusBar()->addWidget(sizeText=new QLabel(statusBar()),0,true);
	statusBar()->addWidget(iocText=new QLabel(statusBar()),0,true);
	QToolTip::add(iocText,tr("Index Of Cooperation:\n"
				 "image width in pixels divided by PI"));
	
	modTool=new QToolBar(tr("modulation settings"),this);
	new QLabel(tr("carrier"),modTool);
	QSpinBox* carrier=new QSpinBox(1500,2400,100,modTool);
	carrier->setSuffix(tr("Hz"));
	modTool->addSeparator();
	QToolTip::add(carrier,tr("signal carrier for FM and AM"));
	new QLabel(tr("deviation"),modTool);
	QSpinBox* deviation=new QSpinBox(400,500,10,modTool);
	deviation->setSuffix(tr("Hz"));
	QToolTip::add(deviation, tr("deviation for FM"));
	modTool->addSeparator();
	new QLabel(tr("modulation"),modTool);
	modulation=new QComboBox(false,modTool);
	modulation->insertItem(tr("AM"));
	modulation->insertItem(tr("FM"));
	QToolTip::add(modulation,tr("AM is only used for getting images\n"
				    "from weather satellites together with\n"
				    "a FM receiver. FM together with a\n"
				    "USB (upper side band) transceiver is\n"
				    "the right setting for HF"));

	aptTool=new QToolBar(tr("apt settings"),this);
	new QLabel(tr("apt start"),aptTool);
	QSpinBox* aptStartLength=new QSpinBox(0,20,1,aptTool);
	aptStartLength->setSuffix(tr("s"));
	QToolTip::add(aptStartLength,tr("length of the black/white pattern\n"
					"at the beginning of a facsimile"));
	QSpinBox* aptStartFreq=new QSpinBox(300,675,10,aptTool);
	aptStartFreq->setSuffix(tr("Hz"));
	QToolTip::add(aptStartFreq,tr("frequency of the black/white pattern\n"
				      "at the beginning of a facsimile"));
	aptTool->addSeparator();
	new QLabel(tr("apt stop"),aptTool);
	QSpinBox* aptStopLength=new QSpinBox(0,20,1,aptTool);
	QToolTip::add(aptStopLength,tr("length of the black/white pattern\n"
				       "at the end of a facsimile"));
	aptStopLength->setSuffix(tr("s"));
	QSpinBox* aptStopFreq=new QSpinBox(300,675,10,aptTool);
	aptStopFreq->setSuffix(tr("Hz"));
	QToolTip::add(aptStopFreq,tr("frequency of the black/white pattern\n"
				     "at the end of a facsimile"));
	faxTool=new QToolBar(tr("facsimile settings"),this);
	new QLabel(tr("lpm"),faxTool);
	QSpinBox* lpm=new QSpinBox(60,360,10,faxTool);
	QToolTip::add(lpm,tr("lines per minute"));
	faxTool->addSeparator();
	new QLabel(tr("phasing lines"),faxTool);
	QSpinBox* phaseLines=new QSpinBox(0,50,1,faxTool);
	QToolTip::add(phaseLines,tr("phasing lines mark the beginning\n"
				    "of a line and the speed (lpm)"));
	invertPhase=new QComboBox(false,faxTool);
	invertPhase->insertItem(tr("normal"));
	invertPhase->insertItem(tr("inverted"));
	QToolTip::add(invertPhase,tr("normal means 2.5% white, 95% black\n"
				     "and again 2.5% white"));
	faxTool->addSeparator();
	color=new QComboBox(false,faxTool);
	color->insertItem(tr("mono"));
	color->insertItem(tr("color"));
	QToolTip::add(color,
		      tr("In color mode each line\n"
			 "is split in three lines:\n"
			 "red, green and blue."));

	// configuration values
	connect(config,SIGNAL(carrier(int)),carrier,SLOT(setValue(int)));
	connect(carrier,SIGNAL(valueChanged(int)),
		config,SLOT(setCarrier(int)));
	connect(config,SIGNAL(carrier(int)),
		faxModulator,SLOT(setCarrier(int)));
	connect(config,SIGNAL(carrier(int)),
		faxDemodulator,SLOT(setCarrier(int)));

	connect(config,SIGNAL(deviation(int)),
		deviation,SLOT(setValue(int)));
	connect(deviation,SIGNAL(valueChanged(int)),
		config,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),
		faxModulator,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),
		faxDemodulator,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),ptc,SLOT(setDeviation(int)));

	connect(config,SIGNAL(useFM(bool)),SLOT(setModulation(bool)));
	connect(modulation,SIGNAL(activated(int)),config,SLOT(setUseFM(int)));
	connect(config,SIGNAL(useFM(bool)),faxModulator,SLOT(setFM(bool)));
	connect(config,SIGNAL(useFM(bool)),faxDemodulator,SLOT(setFM(bool)));
	connect(config,SIGNAL(useFM(bool)),ptc,SLOT(setFM(bool)));

	connect(config,SIGNAL(aptStartLength(int)),
		aptStartLength,SLOT(setValue(int)));
	connect(aptStartLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartLength(int)));
	connect(config,SIGNAL(aptStartLength(int)),
		faxTransmitter,SLOT(setAptStartLength(int)));

	connect(config,SIGNAL(aptStartFreq(int)),
		aptStartFreq,SLOT(setValue(int)));
	connect(aptStartFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartFreq(int)));
	connect(config,SIGNAL(aptStartFreq(int)),
		faxTransmitter,SLOT(setAptStartFreq(int)));
	connect(config,SIGNAL(aptStartFreq(int)),
		faxReceiver,SLOT(setAptStartFreq(int)));

	connect(config,SIGNAL(aptStopLength(int)),
		aptStopLength,SLOT(setValue(int)));
	connect(aptStopLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopLength(int)));
	connect(config,SIGNAL(aptStopLength(int)),
		faxTransmitter,SLOT(setAptStopLength(int)));

	connect(config,SIGNAL(aptStopFreq(int)),
		aptStopFreq,SLOT(setValue(int)));
	connect(aptStopFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopFreq(int)));
	connect(config,SIGNAL(aptStopFreq(int)),
		faxTransmitter,SLOT(setAptStopFreq(int)));
	connect(config,SIGNAL(aptStopFreq(int)),
		faxReceiver,SLOT(setAptStopFreq(int)));

	connect(config,SIGNAL(lpm(int)),lpm,SLOT(setValue(int)));
	connect(lpm,SIGNAL(valueChanged(int)),config,SLOT(setLpm(int)));
	connect(config,SIGNAL(lpm(int)),faxTransmitter,SLOT(setLPM(int)));
	connect(config,SIGNAL(lpm(int)),faxReceiver,SLOT(setTxLPM(int)));
	
	connect(config,SIGNAL(phaseLines(int)),
		phaseLines,SLOT(setValue(int)));
	connect(phaseLines,SIGNAL(valueChanged(int)),
		config,SLOT(setPhaseLines(int)));
	connect(config,SIGNAL(phaseLines(int)),
		faxTransmitter,SLOT(setPhasingLines(int)));

	connect(config,SIGNAL(phaseInvert(bool)),SLOT(setPhasingPol(bool)));
	connect(invertPhase,SIGNAL(activated(int)),
		config,SLOT(setPhaseInvert(int)));
	connect(config,SIGNAL(phaseInvert(bool)),
		faxTransmitter,SLOT(setPhasePol(bool)));
	connect(config,SIGNAL(phaseInvert(bool)),
		faxReceiver,SLOT(setPhasePol(bool)));

	connect(config,SIGNAL(color(bool)),SLOT(setColor(bool)));
	connect(color,SIGNAL(activated(int)),config,SLOT(setColor(int)));
	connect(config,SIGNAL(color(bool)),
		faxTransmitter,SLOT(setColor(bool)));
	connect(config,SIGNAL(color(bool)),faxReceiver,SLOT(setColor(bool)));

	connect(config,SIGNAL(autoScroll(bool)),
		faxImage,SLOT(setAutoScroll(bool)));
	connect(config,SIGNAL(autoScroll(bool)), SLOT(setAutoScroll(bool)));

	connect(config,SIGNAL(toolTip(bool)), SLOT(setToolTip(bool)));
	
	connect(this,SIGNAL(loadFile(QString)),faxImage,SLOT(load(QString)));
	connect(this,SIGNAL(saveFile(QString)),faxImage,SLOT(save(QString)));

	connect(faxImage,SIGNAL(sizeUpdated(int, int)),
		SLOT(newImageSize(int, int)));
	connect(faxImage,SIGNAL(sizeUpdated(int,int)),
		faxReceiver,SLOT(setWidth(int)));
	connect(faxReceiver,SIGNAL(newPixel(int, int, int, int)),
		faxImage,SLOT(setPixel(int, int, int,int)));
	connect(this,SIGNAL(imageWidth(int)),
		faxReceiver,SLOT(correctWidth(int)));
	connect(faxReceiver,SIGNAL(scaleImage(int, int)),
		faxImage,SLOT(scale(int, int)));
	connect(faxReceiver, SIGNAL(newSize(int, int, int, int)),
		faxImage, SLOT(resize(int, int, int, int)));

	connect(sound,SIGNAL(newSampleRate(int)),
		faxModulator,SLOT(setSampleRate(int)));
	connect(sound,SIGNAL(newSampleRate(int)),
		faxDemodulator,SLOT(setSampleRate(int)));
	connect(sound,SIGNAL(newSampleRate(int)),
		faxReceiver,SLOT(setSampleRate(int)));
	connect(sound,SIGNAL(newSampleRate(int)),
		faxTransmitter,SLOT(setSampleRate(int)));
	
	connect(file,SIGNAL(newSampleRate(int)),
		faxModulator,SLOT(setSampleRate(int)));
	connect(file,SIGNAL(newSampleRate(int)),
		faxDemodulator,SLOT(setSampleRate(int)));
	connect(file,SIGNAL(newSampleRate(int)),
		faxReceiver,SLOT(setSampleRate(int)));
	connect(file,SIGNAL(newSampleRate(int)),
		faxTransmitter,SLOT(setSampleRate(int)));
	
	connect(ptc,SIGNAL(newSampleRate(int)),
		faxReceiver,SLOT(setSampleRate(int)));
	connect(ptc,SIGNAL(newSampleRate(int)),
		faxTransmitter,SLOT(setSampleRate(int)));
	
	// transmission
	connect(faxTransmitter,SIGNAL(start()),transmitDialog,SLOT(start()));
	connect(faxTransmitter,SIGNAL(start()),	ptt,SLOT(set()));
	connect(faxTransmitter,SIGNAL(start()),SLOT(disableControls()));
	connect(faxTransmitter,SIGNAL(phasing()),
		transmitDialog,SLOT(phasing()));
	connect(faxTransmitter,SIGNAL(imageLine(int)),
		transmitDialog,SLOT(imageLine(int)));
	connect(faxTransmitter,SIGNAL(aptStop()),
		transmitDialog,SLOT(aptStop()));
	connect(faxTransmitter,SIGNAL(end()),SLOT(endTransmission()));
	connect(sound,SIGNAL(deviceClosed()),transmitDialog,SLOT(hide()));
	connect(ptc,SIGNAL(deviceClosed()),transmitDialog,SLOT(hide()));
	connect(sound,SIGNAL(deviceClosed()),SLOT(enableControls()));
	connect(ptc,SIGNAL(deviceClosed()),SLOT(enableControls()));
	connect(sound,SIGNAL(deviceClosed()),ptt,SLOT(release()));
	connect(transmitDialog,SIGNAL(cancelClicked()),
		faxTransmitter,SLOT(doAptStop()));

	// reception
	connect(faxReceiver,SIGNAL(startReception()),
		receiveDialog,SLOT(aptStart()));
	connect(faxReceiver,SIGNAL(startReception()),SLOT(disableControls()));
	connect(faxReceiver,SIGNAL(startReception()),
		receiveDialog,SLOT(show()));
	connect(sound, SIGNAL(data(signed short*,unsigned int)),
		receiveDialog, SLOT(samples(signed short*,unsigned int)));
	connect(faxReceiver,SIGNAL(aptFound(int)),
		receiveDialog,SLOT(apt(int)));
	connect(faxReceiver,SIGNAL(startingPhasing()),
		receiveDialog,SLOT(phasing()));
	connect(faxReceiver,SIGNAL(phasingLine(double)),
		receiveDialog,SLOT(phasingLine(double)));
	connect(faxReceiver,SIGNAL(imageStarts()),
		receiveDialog,SLOT(disableSkip()));
	connect(faxReceiver,SIGNAL(imageRow(int)),
		receiveDialog,SLOT(imageRow(int)));
	connect(faxReceiver,SIGNAL(end()),SLOT(endReception()));
	connect(faxReceiver,SIGNAL(end()),SLOT(enableControls()));
	connect(faxReceiver,SIGNAL(end()),receiveDialog,SLOT(hide()));
	connect(receiveDialog,SIGNAL(skipClicked()),faxReceiver,SLOT(skip()));
	connect(receiveDialog,SIGNAL(cancelClicked()),
		faxReceiver,SLOT(endReception()));

	// correction
	connect(faxReceiver,SIGNAL(bufferNotEmpty(bool)),
		SLOT(setImageAdjust(bool)));
	connect(faxImage,SIGNAL(newImage()), 
		faxReceiver,SLOT(releaseBuffer()));
	connect(faxReceiver,SIGNAL(redrawStarts()),SLOT(disableControls()));

	connect(this,SIGNAL(correctSlant()),faxImage,SLOT(correctSlant()));
	connect(faxImage,SIGNAL(widthAdjust(double)),
		faxReceiver,SLOT(correctLPM(double)));
	connect(this,SIGNAL(correctBegin()),faxImage,SLOT(correctBegin()));
	connect(correctDialog,SIGNAL(cancelClicked()),SLOT(enableControls()));

	QPopupMenu* fileMenu=new QPopupMenu(this);
	fileMenu->insertItem(tr("&Open"),this,SLOT(load()));
	fileMenu->insertItem(tr("&Save"),this,SLOT(save()));
	fileMenu->insertItem(tr("&Quick save as PNG"),this,SLOT(quickSave()));
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
	imageMenu=new QPopupMenu(this);
	imageMenu->insertItem(tr("&Adjust IOC"),
			      this,SLOT(doScaleDialog()));
	imageMenu->insertItem(tr("Adjust IOC tp &288"),
			      faxReceiver,SLOT(correctToIOC288()));
	imageMenu->insertItem(tr("Adjust IOC to &576"),
			      faxReceiver,SLOT(correctToIOC576()));
	imageMenu->insertSeparator();
	slantID=imageMenu->insertItem(tr("slant correction"),
				      this,SLOT(slantWaitFirst()));
	colDrawID=imageMenu->insertItem(tr("redraw as color facsimile")
					,this,SLOT(redrawColor()));
	monoDrawID=imageMenu->insertItem(tr("redraw as mono facsimile"),
					 this,SLOT(redrawMono()));
	imageMenu->insertSeparator();
	imageMenu->insertItem(tr("shift colors (R->G,G->B,B->R)"),
			      faxImage,SLOT(shiftCol1()));
	imageMenu->insertItem(tr("shift colors (R->B,G->R,B->G)"),
			      faxImage,SLOT(shiftCol2()));
	imageMenu->insertItem(tr("set beginning of line"),
			      this,SLOT(setBegin()));
	optionsMenu=new QPopupMenu(this);
	optionsMenu->insertItem(tr("device settings"),
				this,SLOT(doOptionsDialog()));
	optionsMenu->insertSeparator();
	pttID=optionsMenu->
		insertItem(tr("key PTT while transmitting with DSP"),
			   this,SLOT(changePTT()));
	optionsMenu->setItemChecked(pttID,config->getKeyPTT());
	scrollID=optionsMenu->
		insertItem(tr("automatic scroll to last received line"),
			   this,SLOT(changeScroll()));
	toolTipID=optionsMenu->
		insertItem(tr("show tool tips"),
			   this,SLOT(changeToolTip()));
	QPopupMenu* helpMenu=new QPopupMenu(this);
	helpMenu->insertItem(tr("&Help"),this,SLOT(help()));
	helpMenu->insertSeparator();
	helpMenu->insertItem(tr("&About HamFax"),this,SLOT(about()));
	helpMenu->insertItem(tr("About &QT"),this,SLOT(aboutQT()));

	menuBar()->insertItem(tr("&File"),fileMenu);
	menuBar()->insertItem(tr("&Transmit"),transmitMenu);
	menuBar()->insertItem(tr("&Receive"),receiveMenu);
	menuBar()->insertItem(tr("&Image"),imageMenu);
	menuBar()->insertItem(tr("&Options"),optionsMenu);
	menuBar()->insertSeparator();
	menuBar()->insertItem(tr("&Help"),helpMenu);

	connect(transmitMenu,SIGNAL(activated(int)),SLOT(initTransmit(int)));
	connect(receiveMenu,SIGNAL(activated(int)),SLOT(initReception(int)));
	
	faxImage->create(904,904);
	resize(600,440);
	config->readFile();
}

void FaxWindow::help(void)
{
	HelpDialog* helpDialog=new HelpDialog(this);
	helpDialog->setCaption(caption());
	helpDialog->exec();
	delete helpDialog;
}

void FaxWindow::about(void)
{
	QMessageBox::information(this,caption(),
				 tr("HamFax is a QT application for "
				    "transmitting and receiving "
				    "ham radio facsimiles.\n"
				    "Author: Christof Schmitt, DH1CS\n"
				    "License: GPL\n"
				    "Version: %1").arg(version));
}

void FaxWindow::aboutQT(void)
{
	QMessageBox::aboutQt(this,caption());
}

void FaxWindow::load(void)
{
	QString fileName=getFileName(tr("load image"),
				     "*."+QImage::inputFormatList().
				     join(" *.").lower());
	if(!fileName.isEmpty()) {
		emit loadFile(fileName);
	}
}

void FaxWindow::save(void)
{
	QString fileName=getFileName(tr("save image"),
				     "*."+QImage::outputFormatList().
				     join(" *.").lower());
	if(!fileName.isEmpty()) {
		emit saveFile(fileName);
	}
}

void FaxWindow::doOptionsDialog(void)
{
	OptionsDialog* d=new OptionsDialog(this);
	d->devDSPName=config->getDSPDevice();
	d->devPTTName=config->getPTTDevice();
	d->devPTCName=config->getPTCDevice();
	d->init();
	if(d->exec()) {
		config->setDSP(d->devDSPName);
		config->setPTT(d->devPTTName);
		config->setPTC(d->devPTCName);
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
		emit imageWidth(d->width);
	}
	delete d;
}

void FaxWindow::changePTT(void)
{
	if(optionsMenu->isItemChecked(pttID)) {
		optionsMenu->setItemChecked(pttID,false);
		config->setKeyPTT(false);
	} else {
		optionsMenu->setItemChecked(pttID,true);
		config->setKeyPTT(true);
	}
}

void FaxWindow::changeScroll(void)
{
	if(optionsMenu->isItemChecked(scrollID)) {
		optionsMenu->setItemChecked(scrollID,false);
		config->setAutoScroll(false);
	} else {
		optionsMenu->setItemChecked(scrollID,true);
		config->setAutoScroll(true);
	}
}

void FaxWindow::setAutoScroll(bool b)
{
	optionsMenu->setItemChecked(scrollID,b);
}

QString FaxWindow::getFileName(QString caption, QString filter)
{
	QFileDialog* fd=new QFileDialog(this,0,true);
	fd->setSizeGripEnabled(false);
	fd->setCaption(caption);
	fd->setFilter(filter);
	QString s;
	if(fd->exec()) {
		s=fd->selectedFile();
	}
	delete fd;
	return s;
}

void FaxWindow::initTransmit(int item)
{
	try {
		QString fileName;
		switch(interface=item) {
		case FILE:
			fileName=getFileName(tr("output file name"),"*.au");
			if(fileName.isEmpty()) {
				return;
			}
			file->startOutput(fileName);
			connect(file,SIGNAL(next(unsigned int)),
				faxTransmitter,SLOT(doNext(unsigned int)));
			connect(faxTransmitter,
				SIGNAL(data(double*, unsigned int)),
				faxModulator,
				SLOT(modulate(double*, unsigned int)));
			connect(faxModulator,
				SIGNAL(data(signed short*, unsigned int)),
				file,
				SLOT(write(signed short*, unsigned int)));
			break;
		case DSP:
			sound->startOutput();
			connect(sound,SIGNAL(spaceLeft(unsigned int)),
				faxTransmitter,SLOT(doNext(unsigned int)));
			connect(faxTransmitter,
				SIGNAL(data(double*, unsigned int)),
				faxModulator,
				SLOT(modulate(double*, unsigned int)));
			connect(faxModulator,
				SIGNAL(data(signed short*, unsigned int)),
				sound,
				SLOT(write(signed short*, unsigned int)));
			break;
		case SCSPTC:
			ptc->startOutput();
			connect(ptc,SIGNAL(spaceLeft(unsigned int)),
				faxTransmitter,SLOT(doNext(unsigned int)));
			connect(faxTransmitter,
				SIGNAL(data(double*, unsigned int)),
				ptc,SLOT(transmit(double*, unsigned int)));
			break;
		}
		faxTransmitter->startTransmission();
	} catch (Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::endTransmission(void)
{
	switch(interface) {
	case FILE:
		file->end();
		disconnect(sound,SIGNAL(spaceLeft(unsigned int)),
			   faxTransmitter,SLOT(doNext(unsigned int)));
		disconnect(faxTransmitter,
			   SIGNAL(data(double*, unsigned int)),
			   faxModulator,
			   SLOT(modulate(double*, unsigned int)));
		disconnect(faxModulator,
			   SIGNAL(data(signed short*, unsigned int)),
			   sound,
			   SLOT(write(signed short*, unsigned int)));
		break;
	case DSP:
		sound->end();
		disconnect(sound,SIGNAL(spaceLeft(unsigned int)),
			faxTransmitter,SLOT(doNext(unsigned int)));
		disconnect(faxTransmitter,
			SIGNAL(data(double*,unsigned int)),
			faxModulator,
			SLOT(modulate(double*,unsigned int)));
		disconnect(faxModulator,
			SIGNAL(data(signed short*,unsigned int)),
			sound,
			SLOT(write(signed short*,unsigned int)));
		break;
	case SCSPTC:
		ptc->end();
		disconnect(ptc,SIGNAL(spaceLeft(unsigned int)),
			   faxTransmitter,SLOT(doNext(unsigned int)));
		disconnect(faxTransmitter,
			   SIGNAL(data(double*, unsigned int)),
			   ptc,SLOT(transmit(double*, unsigned int)));
	}
}

void FaxWindow::initReception(int item)
{
	try {
		QString fileName;
		switch(interface=item) {
		case FILE:
			fileName=getFileName(tr("input file name"),"*.au");
			if(fileName.isEmpty()) {
				return;
			}
			file->startInput(fileName);
			connect(file,
				SIGNAL(data(signed short*,unsigned int)),
				faxDemodulator,
				SLOT(newSamples(signed short*,unsigned int)));
			connect(faxDemodulator,
				SIGNAL(data(unsigned int*,unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*,unsigned int)));
			break;
		case DSP:
			sound->startInput();
			connect(sound,
				SIGNAL(data(signed short*,unsigned int)),
				faxDemodulator,
				SLOT(newSamples(signed short*,unsigned int)));
			connect(faxDemodulator,
				SIGNAL(data(unsigned int*,unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*,unsigned int)));
			break;
		case SCSPTC:
			ptc->startInput();
			connect(ptc,SIGNAL(data(unsigned int*,unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*, unsigned int)));
			break;
		}
		faxReceiver->init();
	} catch(Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::endReception(void)
{
	switch(interface) {
	case FILE:
		file->end();
		disconnect(file,
			SIGNAL(data(signed short*, unsigned int)),
			faxDemodulator,
			SLOT(newSamples(signed short*, unsigned int)));
		disconnect(faxDemodulator,
			SIGNAL(data(unsigned int*, unsigned int)),
			faxReceiver,
			SLOT(decode(unsigned int*, unsigned int)));
		break;
	case DSP:
		sound->end();
		disconnect(sound,
			SIGNAL(data(signed short*, unsigned int)),
			faxDemodulator,
			SLOT(newSamples(signed short*, unsigned int)));
		disconnect(faxDemodulator,
			SIGNAL(data(unsigned int*, unsigned int)),
			faxReceiver,
			SLOT(decode(unsigned int*, unsigned int)));
		break;
	case SCSPTC:
		ptc->end();
		disconnect(ptc,SIGNAL(data(unsigned int*, unsigned int)),
			   faxReceiver,
			   SLOT(decode(unsigned int*, unsigned int)));
		break;
	}
}

void FaxWindow::closeEvent(QCloseEvent* close)
{
	switch(QMessageBox::information(this,caption(),
					tr("Really exit?"),
					tr("&Exit"),tr("&Don't Exit"))) {
	case 0:
		close->accept();
		config->writeFile();
		break;
	case 1:
		break;
	}
}

void FaxWindow::quickSave(void)
{
	QDateTime dt=QDateTime::currentDateTime();
	QDate date=dt.date();
	QTime time=dt.time();
	emit saveFile(QString().
		      sprintf("%04d-%02d-%02d-%02d-%02d-%02d.png",
			      date.year(),date.month(),date.day(),
			      time.hour(),time.minute(),time.second()));
}

void FaxWindow::newImageSize(int w, int h)
{
	sizeText->setText(QString(tr("image size: %1x%2")).arg(w).arg(h));
	unsigned int ioc=(unsigned int)((double)w/M_PI+0.5);
	iocText->setText(QString(tr("IOC: %1").arg(ioc)));
}

void FaxWindow::setModulation(bool b)
{
	modulation->setCurrentItem(b ? 1 : 0);
}

void FaxWindow::setPhasingPol(bool b)
{
	invertPhase->setCurrentItem(b ? 1 : 0);
}

void FaxWindow::setColor(bool b)
{
	color->setCurrentItem(b ? 1 : 0);
}

void FaxWindow::slantWaitFirst(void)
{
	correctDialog->setText(tr("select first point of vertical line"));
	disableControls();
	correctDialog->show();
	connect(faxImage,SIGNAL(clicked()),this,SLOT(slantWaitSecond()));
}

void FaxWindow::slantWaitSecond(void)
{
	correctDialog->setText(tr("select second point of vertical line"));
	disconnect(faxImage,SIGNAL(clicked()),this,SLOT(slantWaitSecond()));
	connect(faxImage,SIGNAL(clicked()),this,SLOT(slantEnd()));
}

void FaxWindow::slantEnd(void)
{
	correctDialog->hide();
	disconnect(faxImage,SIGNAL(clicked()),this,SLOT(slantEnd()));
	emit correctSlant();
}

void FaxWindow::redrawColor(void)
{
	config->setColor(true);
	faxReceiver->correctLPM(1);
}

void FaxWindow::redrawMono(void)
{
	config->setColor(false);
	faxReceiver->correctLPM(1);
}

void FaxWindow::disableControls(void)
{
	menuBar()->setDisabled(true);
	modTool->setDisabled(true);
	aptTool->setDisabled(true);
	faxTool->setDisabled(true);
}

void FaxWindow::enableControls(void)
{
	menuBar()->setDisabled(false);
	modTool->setDisabled(false);
	aptTool->setDisabled(false);
	faxTool->setDisabled(false);
}

void FaxWindow::setImageAdjust(bool b)
{
	imageMenu->setItemEnabled(slantID,b);
	imageMenu->setItemEnabled(colDrawID,b);
	imageMenu->setItemEnabled(monoDrawID,b);
}

void FaxWindow::setBegin(void)
{
	correctDialog->setText(tr("select beginning of line"));
	disableControls();
	correctDialog->show();
	connect(faxImage,SIGNAL(clicked()),this,SLOT(setBeginEnd()));
}

void FaxWindow::setBeginEnd(void)
{
	correctDialog->hide();
	emit correctBegin();
	disconnect(faxImage,SIGNAL(clicked()),this,SLOT(setBeginEnd()));
	enableControls();
}

void FaxWindow::changeToolTip(void)
{
	setToolTip(!optionsMenu->isItemChecked(toolTipID));
}

void FaxWindow::setToolTip(bool b)
{
	optionsMenu->setItemChecked(toolTipID,b);
	QToolTip::setEnabled(b);
	config->setToolTip(b);
}
