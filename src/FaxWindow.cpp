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

#include "FaxWindow.hpp"
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qstring.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qfontdialog.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <math.h>
#include "Config.hpp"
#include "Error.hpp"
#include "HelpDialog.hpp"
#include "OptionsDialog.hpp"
#include "ReceiveDialog.hpp"
#include "PTT.hpp"

FaxWindow::FaxWindow(const QString& version)
	: QMainWindow(0,0,WDestructiveClose)
{
	setCaption(version);

	// create child objects
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

	Config* config=&Config::instance();

	// create items in status bar
	statusBar()->setSizeGripEnabled(false);
	statusBar()->addWidget(sizeText=new QLabel(statusBar()),0,true);
	statusBar()->addWidget(iocText=new QLabel(statusBar()),0,true);
	QToolTip::add(iocText,tr("Index Of Cooperation:\n"
				 "image width in pixels divided by PI"));
	
	createToolbars();
	createMenubar();

	// how configuration values get to the correct places

	connect(config,SIGNAL(autoScroll(bool)), SLOT(setAutoScroll(bool)));
	connect(this,SIGNAL(autoScroll(bool)), 
		config, SLOT(setAutoScroll(bool)));
	connect(config,SIGNAL(toolTip(bool)), SLOT(setToolTip(bool)));
	connect(this,SIGNAL(toolTip(bool)), config,SLOT(setToolTip(bool)));

	connect(this,SIGNAL(fontSelected(QFont)),config,SLOT(setFont(QFont)));
	
	connect(config,SIGNAL(keyPTT(bool)),SLOT(setUsePTT(bool)));
	connect(this,SIGNAL(usePTT(bool)),config,SLOT(setKeyPTT(bool)));

	connect(this,SIGNAL(loadFile(QString)),faxImage,SLOT(load(QString)));
	connect(this,SIGNAL(saveFile(QString)),faxImage,SLOT(save(QString)));

	connect(faxImage,SIGNAL(sizeUpdated(int, int)),
		SLOT(newImageSize(int, int)));
	connect(faxImage,SIGNAL(sizeUpdated(int,int)),
		faxReceiver,SLOT(setWidth(int)));
	connect(faxImage,SIGNAL(sizeUpdated(int,int)),
		faxTransmitter,SLOT(setImageSize(int,int)));

	connect(faxReceiver,SIGNAL(setPixel(int, int, int, int)),
		faxImage,SLOT(setPixel(int, int, int,int)));
	connect(this,SIGNAL(scaleToWidth(int)),
		faxImage,SLOT(scale(int)));
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
	connect(faxTransmitter,SIGNAL(start()),faxModulator,SLOT(init()));
	connect(sound,SIGNAL(openForWriting()),ptt,SLOT(set()));
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
	connect(file,SIGNAL(deviceClosed()),transmitDialog,SLOT(hide()));
	connect(file,SIGNAL(deviceClosed()),SLOT(enableControls()));
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
		faxDemodulator,SLOT(init()));
	connect(receiveDialog,SIGNAL(skipClicked()),faxReceiver,SLOT(skip()));
	connect(receiveDialog,SIGNAL(cancelClicked()),
		faxReceiver,SLOT(endReception()));
	connect(sound, SIGNAL(data(short*,int)),
		receiveDialog, SLOT(samples(short*,int)));
	connect(file, SIGNAL(data(short*,int)),
		receiveDialog,SLOT(samples(short*,int)));
	connect(faxDemodulator, SIGNAL(data(int*,int)),
		receiveDialog, SLOT(imageData(int*,int)));
	connect(ptc,SIGNAL(data(int*,int)),
		receiveDialog, SLOT(imageData(int*,int)));
	connect(faxReceiver,SIGNAL(aptFound(int)),
		receiveDialog,SLOT(apt(int)));
	connect(faxReceiver,SIGNAL(startingPhasing()),
		receiveDialog,SLOT(phasing()));
	connect(faxReceiver,SIGNAL(phasingLine(double)),
		receiveDialog,SLOT(phasingLine(double)));
	connect(faxReceiver,SIGNAL(imageStarts()),
		receiveDialog,SLOT(disableSkip()));
	connect(faxReceiver,SIGNAL(row(int)),
		receiveDialog,SLOT(imageRow(int)));
	connect(faxReceiver,SIGNAL(end()),SLOT(endReception()));
	connect(faxReceiver,SIGNAL(end()),SLOT(enableControls()));
	connect(faxReceiver,SIGNAL(end()),receiveDialog,SLOT(hide()));

	// correction
	connect(faxReceiver,SIGNAL(bufferNotEmpty(bool)),
		SLOT(setImageAdjust(bool)));
	connect(faxImage,SIGNAL(newImage()),faxReceiver,SLOT(releaseBuffer()));
	connect(faxReceiver,SIGNAL(redrawStarts()),SLOT(disableControls()));

	connect(this,SIGNAL(correctSlant()),faxImage,SLOT(correctSlant()));
	connect(faxImage,SIGNAL(widthAdjust(double)),
		faxReceiver,SLOT(correctLPM(double)));
	connect(this,SIGNAL(correctBegin()),faxImage,SLOT(correctBegin()));
	connect(correctDialog,SIGNAL(cancelClicked()),SLOT(enableControls()));
	connect(this,SIGNAL(newWidth(int)),
		faxReceiver,SLOT(correctWidth(int)));
	connect(faxReceiver,SIGNAL(imageWidth(int)),
		faxImage,SLOT(setWidth(int)));

	
	faxImage->create(904,904);
	resize(600,440);
}

void FaxWindow::createMenubar(void)
{
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
	connect(transmitMenu,SIGNAL(activated(int)),SLOT(initTransmit(int)));

	QPopupMenu* receiveMenu=new QPopupMenu(this);
	receiveMenu->insertItem(tr("Receive from d&sp"),DSP);
	receiveMenu->insertItem(tr("Receive from f&ile"),FILE);
	receiveMenu->insertItem(tr("Receive from P&TC"),SCSPTC);
	connect(receiveMenu,SIGNAL(activated(int)),SLOT(initReception(int)));

	imageMenu=new QPopupMenu(this);
	imageMenu->insertItem(tr("&Adjust IOC (change width)"),
			      this,SLOT(adjustIOC()));
	imageMenu->insertItem(tr("&Scale to IOC (scale whole image)"),
			      this,SLOT(scaleToIOC()));
	imageMenu->insertSeparator();
	slantID=imageMenu->insertItem(tr("slant correction"),
				      this,SLOT(slantWaitFirst()));
	colDrawID=imageMenu->insertItem(tr("redraw as color facsimile")
					,this,SLOT(redrawColor()));
	monoDrawID=imageMenu->insertItem(tr("redraw as mono facsimile"),
					 this,SLOT(redrawMono()));
	imageMenu->insertSeparator();
	imageMenu->insertItem(tr("shift colors (R->G,G->B,B->R)"),
			      faxImage,SLOT(shiftColors()));
	imageMenu->insertItem(tr("set beginning of line"),
			      this,SLOT(setBegin()));
	optionsMenu=new QPopupMenu(this);
	optionsMenu->insertItem(tr("device settings"),this,SLOT(doOptions()));
	optionsMenu->insertItem(tr("select font"),this,SLOT(selectFont()));
	optionsMenu->insertSeparator();
	pttID=optionsMenu->
		insertItem(tr("key PTT while transmitting with DSP"),
			   this,SLOT(changePTT()));
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
}

void FaxWindow::createToolbars(void)
{
	Config* config=&Config::instance();
	modTool=new QToolBar(tr("modulation settings"),this);
	new QLabel(tr("carrier"),modTool);
	QSpinBox* carrier=new QSpinBox(800,2400,100,modTool);
	connect(config,SIGNAL(carrier(int)),carrier,SLOT(setValue(int)));
	connect(carrier,SIGNAL(valueChanged(int)),
		config,SLOT(setCarrier(int)));
	carrier->setSuffix(tr("Hz"));
	modTool->addSeparator();
	QToolTip::add(carrier,tr("signal carrier for FM and AM"));
	
	new QLabel(tr("deviation"),modTool);
	QSpinBox* deviation=new QSpinBox(400,500,10,modTool);
	connect(config,SIGNAL(deviation(int)),
		deviation,SLOT(setValue(int)));
	connect(deviation,SIGNAL(valueChanged(int)),
		config,SLOT(setDeviation(int)));
	deviation->setSuffix(tr("Hz"));
	QToolTip::add(deviation, tr("deviation for FM"));
	modTool->addSeparator();
	
	new QLabel(tr("modulation"),modTool);
	modulation=new QComboBox(false,modTool);
	connect(config,SIGNAL(useFM(bool)),SLOT(setModulation(bool)));
	connect(modulation,SIGNAL(activated(int)),config,SLOT(setUseFM(int)));
	modulation->insertItem(tr("AM"));
	modulation->insertItem(tr("FM"));
	QToolTip::add(modulation,tr("AM is only used for getting images\n"
				    "from weather satellites together with\n"
				    "a FM receiver. FM together with a\n"
				    "USB (upper side band) transceiver is\n"
				    "the right setting for HF"));
	modTool->addSeparator();

	new QLabel(tr("filter"),modTool);
	filter=new QComboBox(false,modTool);
	connect(config,SIGNAL(filter(int)),SLOT(setFilter(int)));
	connect(filter,SIGNAL(activated(int)),config,SLOT(setFilter(int)));
	filter->insertItem(tr("narrow"));
	filter->insertItem(tr("middle"));
	filter->insertItem(tr("wide"));
	QToolTip::add(filter,tr("bandwidth of the software demodulator"));

	aptTool=new QToolBar(tr("apt settings"),this);
	new QLabel(tr("apt start"),aptTool);
	QSpinBox* aptStartLength=new QSpinBox(0,20,1,aptTool);
	connect(config,SIGNAL(aptStartLength(int)),
		aptStartLength,SLOT(setValue(int)));
	connect(aptStartLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartLength(int)));
	aptStartLength->setSuffix(tr("s"));
	QToolTip::add(aptStartLength,tr("length of the black/white pattern\n"
					"at the beginning of a facsimile"));

	QSpinBox* aptStartFreq=new QSpinBox(300,675,10,aptTool);
	connect(config,SIGNAL(aptStartFreq(int)),
		aptStartFreq,SLOT(setValue(int)));
	connect(aptStartFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartFreq(int)));
	aptStartFreq->setSuffix(tr("Hz"));
	QToolTip::add(aptStartFreq,tr("frequency of the black/white pattern\n"
				      "at the beginning of a facsimile"));
	aptTool->addSeparator();

	new QLabel(tr("apt stop"),aptTool);
	QSpinBox* aptStopLength=new QSpinBox(0,20,1,aptTool);
	connect(config,SIGNAL(aptStopLength(int)),
		aptStopLength,SLOT(setValue(int)));
	connect(aptStopLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopLength(int)));
	QToolTip::add(aptStopLength,tr("length of the black/white pattern\n"
				       "at the end of a facsimile"));
	aptStopLength->setSuffix(tr("s"));

	QSpinBox* aptStopFreq=new QSpinBox(300,675,10,aptTool);
	connect(config,SIGNAL(aptStopFreq(int)),
		aptStopFreq,SLOT(setValue(int)));
	connect(aptStopFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopFreq(int)));
	aptStopFreq->setSuffix(tr("Hz"));
	QToolTip::add(aptStopFreq,tr("frequency of the black/white pattern\n"
				     "at the end of a facsimile"));
	
	faxTool=new QToolBar(tr("facsimile settings"),this);
	new QLabel(tr("lpm"),faxTool);
	QSpinBox* lpm=new QSpinBox(60,360,10,faxTool);
	connect(config,SIGNAL(lpm(int)),lpm,SLOT(setValue(int)));
	connect(lpm,SIGNAL(valueChanged(int)),config,SLOT(setLpm(int)));
	QToolTip::add(lpm,tr("lines per minute"));
	faxTool->addSeparator();

	new QLabel(tr("phasing lines"),faxTool);
	QSpinBox* phaseLines=new QSpinBox(0,50,1,faxTool);
	connect(config,SIGNAL(phaseLines(int)),
		phaseLines,SLOT(setValue(int)));
	connect(phaseLines,SIGNAL(valueChanged(int)),
		config,SLOT(setPhaseLines(int)));
	QToolTip::add(phaseLines,tr("phasing lines mark the beginning\n"
				    "of a line and the speed (lpm)"));

	invertPhase=new QComboBox(false,faxTool);
	connect(config,SIGNAL(phaseInvert(bool)),SLOT(setPhasingPol(bool)));
	connect(invertPhase,SIGNAL(activated(int)),
		config,SLOT(setPhaseInvert(int)));
	invertPhase->insertItem(tr("normal"));
	invertPhase->insertItem(tr("inverted"));
	QToolTip::add(invertPhase,tr("normal means 2.5% white, 95% black\n"
				     "and again 2.5% white"));

	faxTool->addSeparator();
	colorBox=new QComboBox(false,faxTool);
	colorBox->insertItem(tr("mono"));
	colorBox->insertItem(tr("color"));
	connect(config,SIGNAL(color(bool)),SLOT(setColor(bool)));
	connect(colorBox,SIGNAL(activated(int)),config,SLOT(setColor(int)));
	connect(this,SIGNAL(color(bool)),config,SLOT(setColor(bool)));
	QToolTip::add(colorBox,
		      tr("In color mode each line\n"
			 "is split in three lines:\n"
			 "red, green and blue."));
}

void FaxWindow::help(void)
{
	HelpDialog* helpDialog=new HelpDialog(this);
	helpDialog->exec();
	delete helpDialog;
}

void FaxWindow::about(void)
{
	QMessageBox::information(
		this,caption(),
		tr("HamFax is a Qt application for transmitting and receiving"
		   "\nham radio facsimiles and weather broadcasts.\n\n"
		   "Author: Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>"
		   "\n\nThe demodulator is taken from ACfax"
		   " by Andreas Czechanowski, DL4SDC"
		   "\n\nLicense: GNU General Public License"
		   "\nVersion: %1").arg(caption()));
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

void FaxWindow::changePTT(void)
{
	bool b = optionsMenu->isItemChecked(pttID) ? false : true;
	optionsMenu->setItemChecked(pttID,b);
	emit usePTT(b);
}

void FaxWindow::setUsePTT(bool b)
{
	optionsMenu->setItemChecked(pttID,b);
}

void FaxWindow::changeScroll(void)
{
	bool b = optionsMenu->isItemChecked(scrollID) ? false : true;
	optionsMenu->setItemChecked(scrollID,b);
	emit autoScroll(b);
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
			connect(file,SIGNAL(next(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,	SIGNAL(data(double*, int)),
				faxModulator, SLOT(modulate(double*, int)));
			connect(faxModulator, SIGNAL(data(short*, int)),
				file, SLOT(write(short*, int)));
			break;
		case DSP:
			sound->startOutput();
			connect(sound,SIGNAL(spaceLeft(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,	SIGNAL(data(double*, int)),
				faxModulator, SLOT(modulate(double*, int)));
			connect(faxModulator, SIGNAL(data(short*, int)),
				sound, SLOT(write(short*, int)));
			break;
		case SCSPTC:
			ptc->startOutput();
			connect(ptc,SIGNAL(spaceLeft(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,SIGNAL(data(double*, int)),
				ptc,SLOT(transmit(double*, int)));
			break;
		}
		faxTransmitter->startTransmission();
	} catch (Error e) {
		QMessageBox::warning(this,caption(),e.getText());
	}
}

void FaxWindow::endTransmission(void)
{
	switch(interface) {
	case FILE:
		file->end();
		disconnect(sound,SIGNAL(spaceLeft(int)),
			   faxTransmitter,SLOT(doNext(int)));
		disconnect(faxTransmitter, SIGNAL(data(double*, int)),
			   faxModulator, SLOT(modulate(double*, int)));
		disconnect(faxModulator, SIGNAL(data(short*, int)),
			   file, SLOT(write(short*, int)));
		break;
	case DSP:
		sound->end();
		disconnect(sound,SIGNAL(spaceLeft(int)),
			   faxTransmitter,SLOT(doNext(int)));
		disconnect(faxTransmitter, SIGNAL(data(double*,int)),
			faxModulator, SLOT(modulate(double*,int)));
		disconnect(faxModulator, SIGNAL(data(short*,int)),
			sound, SLOT(write(short*,int)));
		break;
	case SCSPTC:
		ptc->end();
		disconnect(ptc,SIGNAL(spaceLeft(int)),
			   faxTransmitter,SLOT(doNext(int)));
		disconnect(faxTransmitter, SIGNAL(data(double*, int)),
			   ptc,SLOT(transmit(double*, int)));
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
			connect(file, SIGNAL(data(short*,int)),
				faxDemodulator, SLOT(newSamples(short*,int)));
			connect(faxDemodulator, SIGNAL(data(int*,int)),
				faxReceiver, SLOT(decode(int*,int)));
			break;
		case DSP:
			sound->startInput();
			connect(sound,SIGNAL(data(short*,int)),
				faxDemodulator,	SLOT(newSamples(short*,int)));
			connect(faxDemodulator,	SIGNAL(data(int*,int)),
				faxReceiver, SLOT(decode(int*,int)));
			break;
		case SCSPTC:
			ptc->startInput();
			connect(ptc,SIGNAL(data(int*,int)),
				faxReceiver, SLOT(decode(int*, int)));
			break;
		}
		faxReceiver->init();
	} catch(Error e) {
		QMessageBox::warning(this,caption(),e.getText());
	}
}

void FaxWindow::endReception(void)
{
	switch(interface) {
	case FILE:
		file->end();
		disconnect(file, SIGNAL(data(short*, int)),
			faxDemodulator, SLOT(newSamples(short*, int)));
		disconnect(faxDemodulator, SIGNAL(data(int*, int)),
			faxReceiver, SLOT(decode(int*, int)));
		break;
	case DSP:
		sound->end();
		disconnect(sound, SIGNAL(data(short*, int)),
			faxDemodulator,	SLOT(newSamples(short*, int)));
		disconnect(faxDemodulator, SIGNAL(data(int*, int)),
			faxReceiver, SLOT(decode(int*, int)));
		break;
	case SCSPTC:
		ptc->end();
		disconnect(ptc,SIGNAL(data(int*, int)),
			   faxReceiver, SLOT(decode(int*, int)));
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
	ioc=static_cast<int>(0.5+w/M_PI);
	iocText->setText(QString(tr("IOC: %1").arg(ioc)));
}

void FaxWindow::setModulation(bool b)
{
	modulation->setCurrentItem(b ? 1 : 0);
}

void FaxWindow::setFilter(int n)
{
	filter->setCurrentItem(n);
}

void FaxWindow::setPhasingPol(bool b)
{
	invertPhase->setCurrentItem(b ? 1 : 0);
}

void FaxWindow::setColor(bool b)
{
	colorBox->setCurrentItem(b ? 1 : 0);
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
	emit color(true);
	faxReceiver->correctLPM(0);
}

void FaxWindow::redrawMono(void)
{
	emit color(false);
	faxReceiver->correctLPM(0);
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
	emit toolTip(b);
}

void FaxWindow::adjustIOC(void)
{
	bool ok;
	int iocNew=QInputDialog::getInteger(caption(),tr("Please enter IOC"),
					    ioc, 204, 576, 1, &ok, this );
	if(ok) {
		emit newWidth(M_PI*iocNew);
	}
}

void FaxWindow::scaleToIOC(void)
{
	bool ok;
	int newIOC=QInputDialog::getInteger(caption(),tr("Please enter IOC"),
					    ioc, 204, 576, 1, &ok, this );
	if(ok) {
		emit scaleToWidth(M_PI*newIOC);
	}
}

void FaxWindow::selectFont(void)
{
	bool ok;
	QFont f=QFontDialog::getFont(&ok,QApplication::font(),this);
	if (ok) {
		QApplication::setFont(f,true);
		emit fontSelected(f);
	}
}

void FaxWindow::doOptions(void)
{
	OptionsDialog* o=new OptionsDialog(this);
	o->exec();
	delete o;
}
