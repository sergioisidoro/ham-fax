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
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qimage.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <math.h>
#include "Error.hpp"
#include "OptionsDialog.hpp"
#include "ScaleDialog.hpp"

FaxWindow::FaxWindow(const QString& version)
	: version(version)
{
	config=new Config(this);
	faxImage=new FaxImage(this);
	faxView=new FaxView(this,faxImage);
	setCentralWidget(faxView);
	faxTransmitter=new FaxTransmitter(this,faxImage);
	faxReceiver=new FaxReceiver(this);
	sound=new Sound(this);
	file=new File(this);
	ptt=new PTT(this);
	ptc=new PTC(this);
	faxModulator=new FaxModulator(this);
	faxDemodulator=new FaxDemodulator(this);
	transmitDialog=new TransmitDialog(this);
	receiveDialog=new ReceiveDialog(this);

	sizeText=new QLabel(statusBar());
	iocText=new QLabel(statusBar());
	statusBar()->setSizeGripEnabled(false);
	statusBar()->addWidget(sizeText,0,true);
	statusBar()->addWidget(iocText,0,true);
	

	modTool=new QToolBar(tr("modulation settings"),this);
	new QLabel(tr("carrier"),modTool);
	QSpinBox* carrier=new QSpinBox(1500,2400,100,modTool);
	carrier->setSuffix(tr("Hz"));
	modTool->addSeparator();
	new QLabel(tr("deviation"),modTool);
	QSpinBox* deviation=new QSpinBox(400,500,10,modTool);
	deviation->setSuffix(tr("Hz"));
	modTool->addSeparator();
	new QLabel(tr("modulation"),modTool);
	modulation=new QComboBox(false,modTool);
	modulation->insertItem(tr("AM"));
	modulation->insertItem(tr("FM"));

	aptTool=new QToolBar(tr("apt settings"),this);
	new QLabel(tr("apt start"),aptTool);
	QSpinBox* aptStartLength=new QSpinBox(0,20,1,aptTool);
	aptStartLength->setSuffix(tr("s"));
	QSpinBox* aptStartFreq=new QSpinBox(300,675,10,aptTool);
	aptStartFreq->setSuffix(tr("Hz"));
	aptTool->addSeparator();
	new QLabel(tr("apt stop"),aptTool);
	QSpinBox* aptStopLength=new QSpinBox(0,20,1,aptTool);
	aptStopLength->setSuffix(tr("s"));
	QSpinBox* aptStopFreq=new QSpinBox(300,675,10,aptTool);
	aptStopFreq->setSuffix(tr("Hz"));

	faxTool=new QToolBar(tr("facsimile settings"),this);
	new QLabel(tr("lpm"),faxTool);
	QSpinBox* lpm=new QSpinBox(60,360,10,faxTool);
	faxTool->addSeparator();
	new QLabel(tr("phasing lines"),faxTool);
	QSpinBox* phaseLines=new QSpinBox(0,50,1,faxTool);
	invertPhase=new QComboBox(false,faxTool);
	invertPhase->insertItem(tr("normal"));
	invertPhase->insertItem(tr("inverted"));
	faxTool->addSeparator();
	color=new QComboBox(false,faxTool);
	color->insertItem(tr("mono"));
	color->insertItem(tr("color"));

	connect(config,SIGNAL(carrier(int)),carrier,SLOT(setValue(int)));
	connect(carrier,SIGNAL(valueChanged(int)),
		config,SLOT(setCarrier(int)));
	connect(config,SIGNAL(carrier(int)),
		faxModulator,SLOT(setCarrier(int)));
	connect(config,SIGNAL(carrier(int)),
		faxDemodulator,SLOT(setCarrier(int)));

	connect(config,SIGNAL(lpm(int)),lpm,SLOT(setValue(int)));
	connect(lpm,SIGNAL(valueChanged(int)),config,SLOT(setLpm(int)));
	connect(config,SIGNAL(lpm(int)),faxTransmitter,SLOT(setLPM(int)));

	connect(config,SIGNAL(aptStartFreq(int)),
		aptStartFreq,SLOT(setValue(int)));
	connect(aptStartFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartFreq(int)));
	connect(config,SIGNAL(aptStartFreq(int)),
		faxTransmitter,SLOT(setAptStartFreq(int)));
	connect(config,SIGNAL(aptStartFreq(int)),
		faxReceiver,SLOT(setAptStartFreq(int)));

	connect(config,SIGNAL(aptStartLength(int)),
		aptStartLength,SLOT(setValue(int)));
	connect(aptStartLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStartLength(int)));
	connect(config,SIGNAL(aptStartLength(int)),
		faxTransmitter,SLOT(setAptStartLength(int)));

	connect(config,SIGNAL(aptStopFreq(int)),
		aptStopFreq,SLOT(setValue(int)));
	connect(aptStopFreq,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopFreq(int)));
	connect(config,SIGNAL(aptStopFreq(int)),
		faxTransmitter,SLOT(setAptStopFreq(int)));
	connect(config,SIGNAL(aptStopFreq(int)),
		faxReceiver,SLOT(setAptStopFreq(int)));

	connect(config,SIGNAL(aptStopLength(int)),
		aptStopLength,SLOT(setValue(int)));
	connect(aptStopLength,SIGNAL(valueChanged(int)),
		config,SLOT(setAptStopLength(int)));
	connect(config,SIGNAL(aptStopLength(int)),
		faxTransmitter,SLOT(setAptStopLength(int)));

	connect(config,SIGNAL(phaseLines(int)),
		phaseLines,SLOT(setValue(int)));
	connect(phaseLines,SIGNAL(valueChanged(int)),
		config,SLOT(setPhaseLines(int)));
	connect(config,SIGNAL(phaseLines(int)),
		faxTransmitter,SLOT(setPhasingLines(int)));

	connect(config,SIGNAL(phaseInvert(bool)),
		this,SLOT(setPhasingPol(bool)));
	connect(invertPhase,SIGNAL(activated(int)),
		config,SLOT(setPhaseInvert(int)));
	connect(config,SIGNAL(phaseInvert(bool)),
		faxTransmitter,SLOT(setPhasePol(bool)));
	connect(config,SIGNAL(phaseInvert(bool)),
		faxReceiver,SLOT(setPhasePol(bool)));

	connect(config,SIGNAL(deviation(int)),
		deviation,SLOT(setValue(int)));
	connect(deviation,SIGNAL(valueChanged(int)),
		config,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),
		faxModulator,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),
		faxDemodulator,SLOT(setDeviation(int)));
	connect(config,SIGNAL(deviation(int)),
		ptc,SLOT(setDeviation(int)));

	connect(config,SIGNAL(useFM(bool)),
		this,SLOT(setModulation(bool)));
	connect(modulation,SIGNAL(activated(int)),
		config,SLOT(setUseFM(int)));
	connect(config,SIGNAL(useFM(bool)),
		faxModulator,SLOT(setFM(bool)));
	connect(config,SIGNAL(useFM(bool)),
		faxDemodulator,SLOT(setFM(bool)));
	connect(config,SIGNAL(useFM(bool)),
		ptc,SLOT(setFM(bool)));

	connect(config,SIGNAL(autoScroll(bool)),
		faxImage,SLOT(setAutoScroll(bool)));
	connect(config,SIGNAL(autoScroll(bool)),
		this,SLOT(setAutoScroll(bool)));

	connect(config,SIGNAL(color(bool)),
		this,SLOT(setColor(bool)));
	connect(color,SIGNAL(activated(int)),
		config,SLOT(setColor(int)));
	connect(config,SIGNAL(color(bool)),
		faxTransmitter,SLOT(setColor(bool)));
	connect(config,SIGNAL(color(bool)),
		faxReceiver,SLOT(setColor(bool)));

	// FaxWindow -- FaxImage -- FaxView
	connect(this,SIGNAL(loadFile(QString)),faxImage,SLOT(load(QString)));
	connect(this,SIGNAL(saveFile(QString)),faxImage,SLOT(save(QString)));
	connect(faxImage,SIGNAL(sizeUpdated(unsigned int, unsigned int)),
		faxView,SLOT(updateView(unsigned int, unsigned int)));
	connect(faxImage,SIGNAL(sizeUpdated(unsigned int, unsigned int)),
		this,SLOT(newImageSize(unsigned int, unsigned int)));
	connect(faxImage,SIGNAL(sizeUpdated(unsigned int,unsigned int)),
		faxReceiver,SLOT(setWidth(unsigned int)));
	connect(faxImage,SIGNAL(contentUpdated(int,int,int,int)),
		faxView,SLOT(update(int,int,int,int)));
	connect(faxImage,SIGNAL(scrollTo(int,int)),
		faxView,SLOT(ensureVisible(int,int)));
	faxImage->create(904,904);

	// FaxReceiver -- FaxImage
	connect(faxReceiver,
		SIGNAL(newImageHeight(unsigned int, unsigned int)),
		faxImage,SLOT(resizeHeight(unsigned int, unsigned int)));
	connect(faxReceiver,SIGNAL(newPixel(unsigned int,unsigned int,
					    unsigned int,unsigned int)),
		faxImage,SLOT(setPixel(unsigned int,unsigned int,
				       unsigned int,unsigned int)));

	// FaxTransmitter -- TransmitDialog
	connect(faxTransmitter,SIGNAL(aptStart()),
		transmitDialog,SLOT(aptStart()));
	connect(faxTransmitter,SIGNAL(phasing()),
		transmitDialog,SLOT(phasing()));
	connect(faxTransmitter,SIGNAL(imageLine(unsigned int)),
		transmitDialog,SLOT(imageLine(unsigned int)));
	connect(faxTransmitter,SIGNAL(aptStop()),
		transmitDialog,SLOT(aptStop()));
	connect(transmitDialog,SIGNAL(cancelClicked()),
		this,SLOT(endTransmission()));

	// FaxReceiver -- ReceiveDialog
	connect(faxReceiver,SIGNAL(aptFound(unsigned int)),
		receiveDialog,SLOT(apt(unsigned int)));
	connect(faxReceiver,SIGNAL(searchingAptStart()),
		receiveDialog,SLOT(aptStart()));
	connect(faxReceiver,SIGNAL(startingPhasing()),
		receiveDialog,SLOT(phasing()));
	connect(faxReceiver,SIGNAL(phasingLine(double)),
		receiveDialog,SLOT(phasingLine(double)));
	connect(receiveDialog,SIGNAL(skipClicked()),
		faxReceiver,SLOT(startPhasing()));
	connect(faxReceiver,SIGNAL(imageRow(unsigned int)),
		receiveDialog,SLOT(imageRow(unsigned int)));
	connect(receiveDialog,SIGNAL(cancelClicked()),
		faxReceiver,SLOT(endReception()));
	connect(faxReceiver,SIGNAL(receptionEnded()),
		this,SLOT(endReception()));

	connect(faxView,SIGNAL(clicked(const QPoint&)),
		faxImage,SLOT(setSlantPoint(const QPoint&)));
	connect(this,SIGNAL(correctSlant()),faxImage,SLOT(correctSlant()));

	buildMenuBar();
	config->readFile();
}

void FaxWindow::buildMenuBar(void)
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

	QPopupMenu* receiveMenu=new QPopupMenu(this);
	receiveMenu->insertItem(tr("Receive from d&sp"),DSP);
	receiveMenu->insertItem(tr("Receive from f&ile"),FILE);
	receiveMenu->insertItem(tr("Receive from P&TC"),SCSPTC);

	QPopupMenu* imageMenu=new QPopupMenu(this);
	imageMenu->insertItem(tr("&Scale image / adjust IOC"),
			      this,SLOT(doScaleDialog()));
	imageMenu->insertItem(tr("Scale image to IOC &288"),
			      faxImage,SLOT(scaleToIOC288()));
	imageMenu->insertItem(tr("Scale image to IOC &576"),
			      faxImage,SLOT(scaleToIOC576()));
	imageMenu->insertSeparator();
	imageMenu->insertItem(tr("correct IOC from 576 to 288"),
			      faxImage,SLOT(halfWidth()));
	imageMenu->insertItem(tr("correct IOC from 288 to 576"),
			      faxImage,SLOT(doubleWidth()));
	imageMenu->insertSeparator();
	imageMenu->insertItem(tr("rotate left"),faxImage,SLOT(rotateLeft()));
	imageMenu->insertItem(tr("rotate right"),
			      faxImage,SLOT(rotateRight()));
	imageMenu->insertSeparator();
	imageMenu->insertItem(tr("slant correction"),
			      this,SLOT(slantWaitFirst()));

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

	QPopupMenu* helpMenu=new QPopupMenu(this);
	helpMenu->insertItem(tr("&About"),this,SLOT(about()));
	helpMenu->insertItem(tr("About &QT"),this,SLOT(aboutQT()));

	menuBar()->insertItem(tr("&File"),fileMenu);
	menuBar()->insertItem(tr("&Transmit"),transmitMenu);
	menuBar()->insertItem(tr("&Receive"),receiveMenu);
	menuBar()->insertItem(tr("&Image"),imageMenu);
	menuBar()->insertItem(tr("&Options"),optionsMenu);
	menuBar()->insertSeparator();
	menuBar()->insertItem(tr("&Help"),helpMenu);

	connect(transmitMenu,SIGNAL(activated(int)),
		this,SLOT(initTransmit(int)));
	connect(receiveMenu,SIGNAL(activated(int)),
		this,SLOT(initReception(int)));
}

void FaxWindow::about(void)
{
	QMessageBox::information(this,this->caption(),
				 tr("HamFax is a QT application for "
				    "transmitting and receiving "
				    "ham radio facsimiles.\n"
				    "Author: Christof Schmitt, DH1CS\n"
				    "License: GPL\n"
				    "Version: %1").arg(version));
}

void FaxWindow::aboutQT(void)
{
	QMessageBox::aboutQt(this,this->caption());
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
		faxImage->scale(d->width,d->height);
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
			sampleRate=5760;
			ptc->open();
			connect(ptc,SIGNAL(spaceLeft(unsigned int)),
				faxTransmitter,SLOT(doNext(unsigned int)));
			connect(faxTransmitter,
				SIGNAL(data(double*, unsigned int)),
				ptc,SLOT(transmit(double*, unsigned int)));
			break;
		}
		faxModulator->setSampleRate(sampleRate);
		faxTransmitter->setSampleRate(sampleRate);
		faxTransmitter->startTransmission();
		menuBar()->setDisabled(true);
		modTool->setDisabled(true);
		aptTool->setDisabled(true);
		faxTool->setDisabled(true);
		transmitDialog->show();

	} catch (Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::endTransmission(void)
{
	switch(interface) {
	case FILE:
		file->close();
		break;
	case DSP:
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
		ptt->set(false);
		sound->close();
		ptt->closeDevice();
		break;
	case SCSPTC:
		ptc->close();
		disconnect(ptc,SIGNAL(spaceLeft(unsigned int)),
			   faxTransmitter,SLOT(doNext(unsigned int)));
		disconnect(faxTransmitter,
			   SIGNAL(data(double*, unsigned int)),
			   ptc,SLOT(transmit(double*, unsigned int)));
	}
	menuBar()->setDisabled(false);
	modTool->setDisabled(false);
	aptTool->setDisabled(false);
	faxTool->setDisabled(false);
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
			connect(file,
				SIGNAL(data(signed short*, unsigned int)),
				faxDemodulator,
				SLOT(newSamples(signed short*, unsigned int)));
			connect(faxDemodulator,
				SIGNAL(data(unsigned int*, unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*, unsigned int)));
			break;
		case DSP:
			sampleRate=8000;
			sound->openInput(sampleRate);
			connect(sound,
				SIGNAL(data(signed short*, unsigned int)),
				faxDemodulator,
				SLOT(newSamples(signed short*, unsigned int)));
			connect(faxDemodulator,
				SIGNAL(data(unsigned int*, unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*, unsigned int)));
			break;
		case SCSPTC:
			sampleRate=5760;
			ptc->open();
			connect(ptc,SIGNAL(data(unsigned int*, unsigned int)),
				faxReceiver,
				SLOT(decode(unsigned int*, unsigned int)));
			break;
		}
		faxDemodulator->setSampleRate(sampleRate);
		faxReceiver->setSampleRate(sampleRate);
		faxReceiver->init();
		menuBar()->setDisabled(true);
		modTool->setDisabled(true);
		aptTool->setDisabled(true);
		faxTool->setDisabled(true);
		receiveDialog->show();
	} catch(Error e) {
		QMessageBox::warning(this,tr("error"),e.getText());
	}
}

void FaxWindow::endReception(void)
{
	switch(interface) {
	case FILE:
		file->close();
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
		sound->close();
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
		ptc->close();
		connect(ptc,SIGNAL(data(unsigned int*, unsigned int)),
			faxReceiver,
			SLOT(decode(unsigned int*, unsigned int)));
		break;
	}
	menuBar()->setDisabled(false);
	modTool->setDisabled(false);
	aptTool->setDisabled(false);
	faxTool->setDisabled(false);
	receiveDialog->hide();
}

void FaxWindow::closeEvent(QCloseEvent* close)
{
	switch(QMessageBox::information(this,this->caption(),
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

void FaxWindow::newImageSize(unsigned int w, unsigned int h)
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
	slantDialog=new QMessageBox(this->caption(),
				    tr("select first point of vertical line"),
				    QMessageBox::Information,
				    QMessageBox::Cancel,
				    QMessageBox::NoButton,
				    QMessageBox::NoButton,
				    this,0,false);
	slantDialog->show();
	connect(faxView,SIGNAL(clicked()),this,SLOT(slantWaitSecond()));
}

void FaxWindow::slantWaitSecond(void)
{
	slantDialog->setText(tr("select second point of vertical line"));
	disconnect(faxView,SIGNAL(clicked()),this,SLOT(slantWaitSecond()));
	connect(faxView,SIGNAL(clicked()),this,SLOT(slantEnd()));
}

void FaxWindow::slantEnd(void)
{
	slantDialog->hide();
	disconnect(faxView,SIGNAL(clicked()),this,SLOT(slantEnd()));
	emit correctSlant();
	delete slantDialog;
}
