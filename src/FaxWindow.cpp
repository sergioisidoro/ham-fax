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
#include "Config.hpp"
#include "Error.hpp"
#include "HelpDialog.hpp"
#include "OptionsDialog.hpp"
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
#include <cmath>

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
	faxModulator=new FaxModulator(this);
	faxDemodulator=new FaxDemodulator(this);
	transmitDialog=new TransmitDialog(this);
	receiveDialog=new ReceiveDialog(this);
	correctDialog=new CorrectDialog(this);

	createStatusBar();
	createToolbars();
	createMenubar();

	connect(faxImage,SIGNAL(sizeUpdated(int, int)),
		SLOT(newImageSize(int, int)));
	connect(faxImage,SIGNAL(sizeUpdated(int,int)),
		faxReceiver,SLOT(setWidth(int)));
	connect(faxImage,SIGNAL(sizeUpdated(int,int)),
		faxTransmitter,SLOT(setImageSize(int,int)));

	connect(faxReceiver,SIGNAL(setPixel(int, int, int, int)),
		faxImage,SLOT(setPixel(int, int, int,int)));
	connect(faxReceiver, SIGNAL(newSize(int, int, int, int)),
		faxImage, SLOT(resize(int, int, int, int)));

	// transmission
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
	connect(transmitDialog,SIGNAL(cancelClicked()),
		faxTransmitter,SLOT(doAptStop()));

	// reception
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
	connect(faxImage,SIGNAL(newImage()),faxReceiver,SLOT(releaseBuffer()));
	connect(faxReceiver,SIGNAL(redrawStarts()),SLOT(disableControls()));
	connect(correctDialog,SIGNAL(cancelClicked()),SLOT(enableControls()));
	connect(faxReceiver,SIGNAL(imageWidth(int)),
		faxImage,SLOT(setWidth(int)));

	connect(faxReceiver,SIGNAL(bufferNotEmpty(bool)),
		SLOT(setImageAdjust(bool)));
	connect(faxImage,SIGNAL(widthAdjust(double)),
		faxReceiver,SLOT(correctLPM(double)));
	
	faxImage->create(904,904);
	resize(600,440);
}

void FaxWindow::createMenubar(void)
{
	Config& config=Config::instance();

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
	optionsMenu->setItemChecked(pttID,config.readBoolEntry("/hamfax/PTT/use"));
	scrollID=optionsMenu->
		insertItem(tr("automatic scroll to last received line"),
			   this,SLOT(changeScroll()));
	optionsMenu->setItemChecked(scrollID,
				    config.readBoolEntry("/hamfax/GUI/autoScroll"));
	toolTipID=optionsMenu->insertItem(tr("show tool tips"),
					  this,SLOT(changeToolTip()));
	bool toolTipEnabled=config.readBoolEntry("/hamfax/GUI/toolTips");
	optionsMenu->setItemChecked(toolTipID,toolTipEnabled);
	QToolTip::setGloballyEnabled(toolTipEnabled);

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
	Config& config=Config::instance();

	modTool=new QToolBar(tr("modulation settings"),this);
	new QLabel(tr("carrier"),modTool);
	QSpinBox* carrier=new QSpinBox(800,2400,100,modTool);
	carrier->setSuffix(tr("Hz"));
	carrier->setValue(config.readNumEntry("/hamfax/modulation/carrier"));
	connect(carrier,SIGNAL(valueChanged(int)),SLOT(setCarrier(int)));
	modTool->addSeparator();
	QToolTip::add(carrier,tr("signal carrier for FM and AM"));
	
	new QLabel(tr("deviation"),modTool);
	QSpinBox* deviation=new QSpinBox(400,500,10,modTool);
	deviation->setValue(config.readNumEntry("/hamfax/modulation/deviation"));
	connect(deviation,SIGNAL(valueChanged(int)),SLOT(setDeviation(int)));
	deviation->setSuffix(tr("Hz"));
	QToolTip::add(deviation, tr("deviation for FM"));
	modTool->addSeparator();
	
	new QLabel(tr("modulation"),modTool);
	modulation=new QComboBox(false,modTool);
	modulation->insertItem(tr("AM"));
	modulation->insertItem(tr("FM"));
	modulation->setCurrentItem(config.readBoolEntry("/hamfax/modulation/FM")
				   ? 1 : 0);
	connect(modulation,SIGNAL(activated(int)),SLOT(setUseFM(int)));
	QToolTip::add(modulation,tr("AM is only used for getting images\n"
				    "from weather satellites together with\n"
				    "a FM receiver. FM together with a\n"
				    "USB (upper side band) transceiver is\n"
				    "the right setting for HF"));
	modTool->addSeparator();

	new QLabel(tr("filter"),modTool);
	filter=new QComboBox(false,modTool);
	filter->insertItem(tr("narrow"));
	filter->insertItem(tr("middle"));
	filter->insertItem(tr("wide"));
	filter->setCurrentItem(config.readNumEntry("/hamfax/modulation/filter"));
	connect(filter,SIGNAL(activated(int)),SLOT(setFilter(int)));
	QToolTip::add(filter,tr("bandwidth of the software demodulator"));

	aptTool=new QToolBar(tr("apt settings"),this);
	new QLabel(tr("apt start"),aptTool);
	QSpinBox* aptStartLength=new QSpinBox(0,20,1,aptTool);
	aptStartLength->setSuffix(tr("s"));
	aptStartLength->setValue(config.readNumEntry("/hamfax/APT/startLength"));
	connect(aptStartLength,SIGNAL(valueChanged(int)),
		SLOT(setAptStartLength(int)));
	QToolTip::add(aptStartLength,tr("length of the black/white pattern\n"
					"at the beginning of a facsimile"));

	QSpinBox* aptStartFreq=new QSpinBox(300,675,10,aptTool);
	aptStartFreq->setSuffix(tr("Hz"));
	aptStartFreq->setValue(config.readNumEntry("/hamfax/APT/startFreq"));
	connect(aptStartFreq,SIGNAL(valueChanged(int)),
		SLOT(setAptStartFreq(int)));
	QToolTip::add(aptStartFreq,tr("frequency of the black/white pattern\n"
				      "at the beginning of a facsimile"));
	aptTool->addSeparator();

	new QLabel(tr("apt stop"),aptTool);
	QSpinBox* aptStopLength=new QSpinBox(0,20,1,aptTool);
	aptStopLength->setSuffix(tr("s"));
	aptStopLength->setValue(config.readNumEntry("/hamfax/APT/stopLength"));
	connect(aptStopLength,SIGNAL(valueChanged(int)),
		SLOT(setAptStopLength(int)));
	QToolTip::add(aptStopLength,tr("length of the black/white pattern\n"
				       "at the end of a facsimile"));

	QSpinBox* aptStopFreq=new QSpinBox(300,675,10,aptTool);
	aptStopFreq->setSuffix(tr("Hz"));
	aptStopFreq->setValue(config.readNumEntry("/hamfax/APT/stopFreq"));
	connect(aptStopFreq,SIGNAL(valueChanged(int)),
		SLOT(setAptStopFreq(int)));
	QToolTip::add(aptStopFreq,tr("frequency of the black/white pattern\n"
				     "at the end of a facsimile"));
	
	faxTool=new QToolBar(tr("facsimile settings"),this);
	new QLabel(tr("lpm"),faxTool);
	QSpinBox* lpm=new QSpinBox(60,360,10,faxTool);
	lpm->setValue(config.readNumEntry("/hamfax/fax/LPM"));
	connect(lpm,SIGNAL(valueChanged(int)),SLOT(setLpm(int)));
	QToolTip::add(lpm,tr("lines per minute"));
	faxTool->addSeparator();

	new QLabel(tr("phasing lines"),faxTool);
	QSpinBox* phaseLines=new QSpinBox(0,50,1,faxTool);
	phaseLines->setValue(config.readNumEntry("/hamfax/phasing/lines"));
	connect(phaseLines,SIGNAL(valueChanged(int)),SLOT(setPhaseLines(int)));
	QToolTip::add(phaseLines,tr("phasing lines mark the beginning\n"
				    "of a line and the speed (lpm)"));

	invertPhase=new QComboBox(false,faxTool);
	invertPhase->insertItem(tr("normal"));
	invertPhase->insertItem(tr("inverted"));
	invertPhase->setCurrentItem(config.readBoolEntry("/hamfax/phasing/invert")
				    ? 1 : 0);
	connect(invertPhase,SIGNAL(activated(int)),SLOT(setPhaseInvert(int)));
	QToolTip::add(invertPhase,tr("normal means 2.5% white, 95% black\n"
				     "and again 2.5% white"));

	faxTool->addSeparator();
	colorBox=new QComboBox(false,faxTool);
	colorBox->insertItem(tr("mono"));
	colorBox->insertItem(tr("color"));
	colorBox->setCurrentItem(config.readBoolEntry("/hamfax/fax/color")?1:0);
	connect(colorBox,SIGNAL(activated(int)),SLOT(setColor(int)));
	QToolTip::add(colorBox,
		      tr("In color mode each line\n"
			 "is split in three lines:\n"
			 "red, green and blue."));
}

void FaxWindow::createStatusBar()
{
	statusBar()->setSizeGripEnabled(false);
	statusBar()->addWidget(sizeText=new QLabel(statusBar()),0,true);
	statusBar()->addWidget(iocText=new QLabel(statusBar()),0,true);
	QToolTip::add(iocText,tr("Index Of Cooperation:\n"
				 "image width in pixels divided by PI"));
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

void FaxWindow::newImageSize(int w, int h)
{
	sizeText->setText(QString(tr("image size: %1x%2")).arg(w).arg(h));
	ioc=static_cast<int>(0.5+w/M_PI);
	iocText->setText(QString(tr("IOC: %1").arg(ioc)));
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
	faxImage->correctSlant();
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

void FaxWindow::setBeginEnd(void)
{
	correctDialog->hide();
	faxImage->correctBegin();
	disconnect(faxImage,SIGNAL(clicked()),this,SLOT(setBeginEnd()));
	enableControls();
}

// slots for menu items

void FaxWindow::load(void)
{
	QString filter="*."+QImage::outputFormatList().join(" *.").lower();
	QString name=QFileDialog::getOpenFileName(".",filter,this,0,caption());
	if(!name.isEmpty()) {
		faxImage->load(name);
	}
}

void FaxWindow::save(void)
{
	QString filter="*."+QImage::outputFormatList().join(" *.").lower();
	QString name=QFileDialog::getSaveFileName(".",filter,this,0,caption());
	if(!name.isEmpty()) {
		faxImage->save(name);
	}
}

void FaxWindow::quickSave(void)
{
	QDateTime dt=QDateTime::currentDateTime();
	QDate date=dt.date();
	QTime time=dt.time();
	QString name;
	name.sprintf("%04d-%02d-%02d-%02d-%02d-%02d.png",
		     date.year(),date.month(),date.day(),
		     time.hour(),time.minute(),time.second());
	faxImage->save(name);
}

void FaxWindow::initTransmit(int item)
{
	try {
		QString fileName;
		int sampleRate;
		switch(interface=item) {
		case FILE:
			fileName=QFileDialog::getSaveFileName(".","*.au",
							      this,0,caption());
			if(fileName.isEmpty()) {
				return;
			}
			sampleRate=file->startOutput(fileName);
			connect(file,SIGNAL(next(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,	SIGNAL(data(double*, int)),
				faxModulator, SLOT(modulate(double*, int)));
			connect(faxModulator, SIGNAL(data(short*, int)),
				file, SLOT(write(short*, int)));
			break;
		case DSP:
			sampleRate=sound->startOutput();
			connect(sound,SIGNAL(spaceLeft(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,	SIGNAL(data(double*, int)),
				faxModulator, SLOT(modulate(double*, int)));
			connect(faxModulator, SIGNAL(data(short*, int)),
				sound, SLOT(write(short*, int)));
			break;
		case SCSPTC:
			sampleRate=ptc->startOutput();
			connect(ptc,SIGNAL(spaceLeft(int)),
				faxTransmitter,SLOT(doNext(int)));
			connect(faxTransmitter,SIGNAL(data(double*, int)),
				ptc,SLOT(transmit(double*, int)));
			break;
		}
		faxTransmitter->start(sampleRate);
		faxModulator->init(sampleRate);
		transmitDialog->start();
		disableControls();
	} catch (Error e) {
		QMessageBox::warning(this,caption(),e.getText());
	}
}

void FaxWindow::initReception(int item)
{
        try {
                int sampleRate;
                QString fileName;
                switch(interface=item) {
                case FILE:
                        fileName=QFileDialog::getOpenFileName(".","*.au",this,
                                                              0,caption());
                        if(fileName.isEmpty()) {
                                return;
                        }
                        sampleRate=file->startInput(fileName);
                        connect(file, SIGNAL(data(short*,int)),
                                faxDemodulator, SLOT(newSamples(short*,int)));
                        connect(faxDemodulator, SIGNAL(data(int*,int)),
                                faxReceiver, SLOT(decode(int*,int)));
                        break;
                case DSP:
                        sampleRate=sound->startInput();
                        connect(sound,SIGNAL(data(short*,int)),
                                faxDemodulator, SLOT(newSamples(short*,int)));
                        connect(faxDemodulator, SIGNAL(data(int*,int)),
                                faxReceiver, SLOT(decode(int*,int)));
                        break;
                case SCSPTC:
                        sampleRate=ptc->startInput();
                        connect(ptc,SIGNAL(data(int*,int)),
                                faxReceiver, SLOT(decode(int*, int)));
                        break;
                }
                faxReceiver->init(sampleRate);
                receiveDialog->aptStart();
                disableControls();
                faxDemodulator->init(sampleRate);
        } catch(Error e) {
                QMessageBox::warning(this,caption(),e.getText());
        }
}

void FaxWindow::adjustIOC(void)
{
	bool ok;
	int iocNew=QInputDialog::getInteger(caption(),tr("Please enter IOC"),
					    ioc, 204, 576, 1, &ok, this );
	if(ok) {
		faxReceiver->correctWidth(M_PI*iocNew);
	}
}

void FaxWindow::scaleToIOC(void)
{
	bool ok;
	int newIOC=QInputDialog::getInteger(caption(),tr("Please enter IOC"),
					    ioc, 204, 576, 1, &ok, this );
	if(ok) {
		faxImage->scale(M_PI*newIOC);
	}
}

void FaxWindow::slantWaitFirst(void)
{
	correctDialog->setText(tr("select first point of vertical line"));
	disableControls();
	correctDialog->show();
	connect(faxImage,SIGNAL(clicked()),this,SLOT(slantWaitSecond()));
}

void FaxWindow::redrawColor(void)
{
	Config::instance().writeEntry("/hamfax/fax/color",true);
	colorBox->setCurrentItem(1);
	faxReceiver->correctLPM(0);
}

void FaxWindow::redrawMono(void)
{
	Config::instance().writeEntry("/hamfax/fax/color",false);
	colorBox->setCurrentItem(0);
	faxReceiver->correctLPM(0);
}

void FaxWindow::setBegin(void)
{
	correctDialog->setText(tr("select beginning of line"));
	disableControls();
	correctDialog->show();
	connect(faxImage,SIGNAL(clicked()),this,SLOT(setBeginEnd()));
}

void FaxWindow::doOptions(void)
{
	OptionsDialog* o=new OptionsDialog(this);
	o->exec();
	delete o;
}

void FaxWindow::selectFont(void)
{
	bool ok;
	QFont f=QFontDialog::getFont(&ok,QApplication::font(),this);
	if (ok) {
		QApplication::setFont(f,true);
		Config::instance().writeEntry("/hamfax/GUI/font",f.rawName());
	}
}

void FaxWindow::changePTT(void)
{
	bool b=!optionsMenu->isItemChecked(pttID);
	optionsMenu->setItemChecked(pttID,b);
	Config::instance().writeEntry("/hamfax/PTT/use",b);
}

void FaxWindow::changeScroll(void)
{
	bool b=!optionsMenu->isItemChecked(scrollID);
	optionsMenu->setItemChecked(scrollID,b);
	Config::instance().writeEntry("/hamfax/GUI/autoScroll",b);
}

void FaxWindow::changeToolTip(void)
{
	bool b=!optionsMenu->isItemChecked(toolTipID);
	optionsMenu->setItemChecked(toolTipID,b);
	Config::instance().writeEntry("/hamfax/GUI/toolTips",b);
	QToolTip::setGloballyEnabled(b);
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

// slots for toolbar objects

void FaxWindow::setCarrier(int c)
{
	Config::instance().writeEntry("/hamfax/modulation/carrier",c);
}

void FaxWindow::setDeviation(int d)
{
	Config::instance().writeEntry("/hamfax/modulation/deviation",d);
}

void FaxWindow::setUseFM(int f)
{
	Config::instance().writeEntry("/hamfax/modulation/FM",f==1);
}

void FaxWindow::setFilter(int n)
{
	filter->setCurrentItem(n);
}

void FaxWindow::setAptStartLength(int l)
{
	Config::instance().writeEntry("/hamfax/APT/startLength",l);
}

void FaxWindow::setAptStartFreq(int f)
{
	Config::instance().writeEntry("/hamfax/APT/startFrequency",f);
}

void FaxWindow::setAptStopLength(int l)
{
	Config::instance().writeEntry("/hamfax/APT/stopLength",l);
}

void FaxWindow::setAptStopFreq(int f)
{
	Config::instance().writeEntry("/hamfax/APT/stopFrequency",f);
}

void FaxWindow::setLpm(int l)
{
	Config::instance().writeEntry("/hamfax/fax/lpm",l);
}

void FaxWindow::setPhaseLines(int l)
{
	Config::instance().writeEntry("/hamfax/fax/phasing/lines",l);
}

void FaxWindow::setPhaseInvert(int i)
{
	Config::instance().writeEntry("/hamfax/fax/phasing/invert",i);
}

void FaxWindow::setColor(int c)
{
	Config::instance().writeEntry("/hamfax/fax/color",c==1);
}
