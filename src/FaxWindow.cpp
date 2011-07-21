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

#include "FaxWindow.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "HelpDialog.hpp"
#include "OptionsDialog.hpp"
#include <qapplication.h>
#include <QFileDialog>
#include <qstring.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qfontdialog.h>
#include <qimage.h>
#include <QImageReader>
#include <QImageWriter>
#include <qinputdialog.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <QLabel>
#include <QCloseEvent>
#include <cmath>

FaxWindow::FaxWindow(const QString& version)
{
	setWindowTitle(version);

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

	restoreGeometry(Config::instance().value("GUI/geometry").toByteArray());
	restoreState(Config::instance().value("GUI/windowState").toByteArray());
}

void FaxWindow::createMenubar(void)
{
	Config& config=Config::instance();

	QMenu* fileMenu = new QMenu(tr("&File"));
	menuBar()->addMenu(fileMenu);
	fileMenu->addAction(tr("&Open"), this, SLOT(load()));
	fileMenu->addAction(tr("&Save"), this, SLOT(save()));
	fileMenu->addAction(tr("&Quick save as PNG"), this, SLOT(quickSave()));
	fileMenu->addSeparator();
	fileMenu->addAction(tr("&Exit"), this, SLOT(close()));

	QMenu* transmitMenu = new QMenu(tr("&Transmit"));
	menuBar()->addMenu(transmitMenu);
	transmitMenu->addAction(tr("Transmit using &dsp"),
				this, SLOT(initTransmitDsp()));
	transmitMenu->addAction(tr("Transmit to &file"),
				this, SLOT(initTransmitFile()));
	transmitMenu->addAction(tr("Transmit using &PTC"),
				this, SLOT(initTransmitPtc()));

	QMenu* receiveMenu = new QMenu(tr("&Receive"));
	menuBar()->addMenu(receiveMenu);
	receiveMenu->addAction(tr("Receive from d&sp"),
			       this, SLOT(initReceptionDsp()));
	receiveMenu->addAction(tr("Receive from f&ile"),
			       this, SLOT(initReceptionFile()));
	receiveMenu->addAction(tr("Receive from P&TC"),
			       this, SLOT(initReceptionPtc()));

	QMenu* imageMenu = new QMenu(tr("&Image"));
	menuBar()->addMenu(imageMenu);
	imageMenu->addAction(tr("&Adjust IOC (change width)"),
			     this, SLOT(adjustIOC()));
	imageMenu->addAction(tr("&Scale to IOC (scale whole image)"),
			     this, SLOT(scaleToIOC()));
	imageMenu->addSeparator();
	slantAction = imageMenu->addAction(tr("slant correction"),
					      this, SLOT(slantWaitFirst()));
	colorDrawAction = imageMenu->addAction(tr("redraw as color facsimile"),
					       this, SLOT(redrawColor()));
	monoDrawAction = imageMenu->addAction(tr("redraw as mono facsimile"),
					      this, SLOT(redrawMono()));
	imageMenu->addSeparator();
	imageMenu->addAction(tr("shift colors (R->G,G->B,B->R)"),
			     faxImage, SLOT(shiftColors()));
	imageMenu->addAction(tr("set beginning of line"),
			     this, SLOT(setBegin()));

	QMenu* optionsMenu = new QMenu(tr("&Options"));
	menuBar()->addMenu(optionsMenu);
	optionsMenu->addAction(tr("device settings"), this, SLOT(doOptions()));
	optionsMenu->addAction(tr("&select font"),this,SLOT(selectFont()));
	optionsMenu->addSeparator();
	pttAction = optionsMenu->addAction(tr("key PTT while transmitting with DSP"),
					   this, SLOT(changePTT()));
	pttAction->setCheckable(true);
	pttAction->setChecked(config.readBoolEntry("/hamfax/PTT/use"));
	scrollAction = optionsMenu->addAction(tr("automatic scroll to last received line"),
					      this, SLOT(changeScroll()));
	scrollAction->setCheckable(true);
	scrollAction->setChecked(config.readBoolEntry("/hamfax/GUI/autoScroll"));
	toolTipAction = optionsMenu->addAction(tr("show tool tips"),
					       this, SLOT(changeToolTip()));
	toolTipAction->setCheckable(true);
	bool toolTipEnabled=config.readBoolEntry("/hamfax/GUI/toolTips");
	toolTipAction->setChecked(toolTipEnabled);

	QMenu* helpMenu = new QMenu(tr("&Help"));
	menuBar()->addSeparator();
	menuBar()->addMenu(helpMenu);
	helpMenu->addAction(tr("&Help"), this, SLOT(help()));
	helpMenu->addSeparator();
	helpMenu->addAction(tr("&About hamfax"), this, SLOT(about()));
	helpMenu->addAction(tr("About &QT"), this, SLOT(aboutQT()));
}

void FaxWindow::createToolbars(void)
{
	Config& config=Config::instance();

	modTool = new QToolBar(tr("modulation settings"),this);
	addToolBar(modTool);
	modTool->addWidget(new QLabel(tr("carrier")));

	QSpinBox* carrier = new QSpinBox();
	modTool->addWidget(carrier);
	carrier->setMinimum(800);
	carrier->setMaximum(2400);
	carrier->setSingleStep(100);
	carrier->setSuffix(tr("Hz"));
	carrier->setValue(config.readNumEntry("/hamfax/modulation/carrier"));
	carrier->setToolTip(tr("signal carrier for FM and AM"));
	connect(carrier,SIGNAL(valueChanged(int)),SLOT(setCarrier(int)));

	modTool->addSeparator();

	modTool->addWidget(new QLabel(tr("deviation")));

	QSpinBox* deviation = new QSpinBox();
	modTool->addWidget(deviation);
	deviation->setMinimum(400);
	deviation->setMaximum(500);
	deviation->setSingleStep(10);
	deviation->setValue(config.readNumEntry("/hamfax/modulation/deviation"));
	deviation->setSuffix(tr("Hz"));
	deviation->setToolTip(tr("deviation for FM"));
	connect(deviation,SIGNAL(valueChanged(int)),SLOT(setDeviation(int)));

	modTool->addSeparator();
	
	modTool->addWidget(new QLabel(tr("modulation")));

	modulation = new QComboBox();
	modTool->addWidget(modulation);
	modulation->addItem(tr("AM"));
	modulation->addItem(tr("FM"));
	modulation->setCurrentIndex(config.readBoolEntry("/hamfax/modulation/FM")
				   ? 1 : 0);
	modulation->setToolTip(tr("AM is only used for getting images\n"
				  "from weather satellites together with\n"
				  "a FM receiver. FM together with a\n"
				  "USB (upper side band) transceiver is\n"
				  "the right setting for HF"));
	connect(modulation, SIGNAL(activated(int)), SLOT(setUseFM(int)));

	modTool->addSeparator();

	modTool->addWidget(new QLabel(tr("filter")));

	filter = new QComboBox();
	modTool->addWidget(filter);
	filter->addItem(tr("narrow"));
	filter->addItem(tr("middle"));
	filter->addItem(tr("wide"));
	filter->setCurrentIndex(config.readNumEntry("/hamfax/modulation/filter"));
	filter->setToolTip(tr("bandwidth of the software demodulator"));
	connect(filter, SIGNAL(activated(int)), SLOT(setFilter(int)));

	addToolBarBreak();

	aptTool = new QToolBar(tr("apt settings"),this);
	addToolBar(aptTool);

	aptTool->addWidget(new QLabel(tr("apt start")));

	QSpinBox* aptStartLength = new QSpinBox();
	aptTool->addWidget(aptStartLength);
	aptStartLength->setMinimum(0);
	aptStartLength->setMaximum(20);
	aptStartLength->setSingleStep(1);
	aptStartLength->setSuffix(tr("s"));
	aptStartLength->setValue(config.readNumEntry("/hamfax/APT/startLength"));
	connect(aptStartLength,SIGNAL(valueChanged(int)),
		SLOT(setAptStartLength(int)));
	aptStartLength->setToolTip(tr("length of the black/white pattern\n"
				      "at the beginning of a facsimile"));

	QSpinBox* aptStartFreq = new QSpinBox();
	aptTool->addWidget(aptStartFreq);
	aptStartFreq->setMinimum(300);
	aptStartFreq->setMaximum(675);
	aptStartFreq->setSingleStep(10);
	aptStartFreq->setSuffix(tr("Hz"));
	aptStartFreq->setValue(config.readNumEntry("/hamfax/APT/startFreq"));
	connect(aptStartFreq,SIGNAL(valueChanged(int)),
		SLOT(setAptStartFreq(int)));
	aptStartFreq->setToolTip(tr("frequency of the black/white pattern\n"
				    "at the beginning of a facsimile"));

	aptTool->addSeparator();

	aptTool->addWidget(new QLabel(tr("apt stop")));
	QSpinBox* aptStopLength = new QSpinBox();
	aptStopLength->setMinimum(0);
	aptStopLength->setMaximum(20);
	aptStopLength->setSingleStep(1);
	aptStopLength->setSuffix(tr("s"));
	aptStopLength->setValue(config.readNumEntry("/hamfax/APT/stopLength"));
	connect(aptStopLength,SIGNAL(valueChanged(int)),
		SLOT(setAptStopLength(int)));
	aptStopLength->setToolTip(tr("length of the black/white pattern\n"
				     "at the end of a facsimile"));

	QSpinBox* aptStopFreq = new QSpinBox();
	aptTool->addWidget(aptStopFreq);
	aptStopFreq->setMinimum(300);
	aptStopFreq->setMaximum(675);
	aptStopFreq->setSingleStep(10);
	aptStopFreq->setSuffix(tr("Hz"));
	aptStopFreq->setValue(config.readNumEntry("/hamfax/APT/stopFreq"));
	connect(aptStopFreq,SIGNAL(valueChanged(int)),
		SLOT(setAptStopFreq(int)));
	aptStopFreq->setToolTip(tr("frequency of the black/white pattern\n"
				   "at the end of a facsimile"));
	
	addToolBarBreak();

	faxTool = new QToolBar(tr("facsimile settings"),this);
	addToolBar(faxTool);

	faxTool->addWidget(new QLabel(tr("lpm")));

	QSpinBox* lpm = new QSpinBox();
	faxTool->addWidget(lpm);
	lpm->setMinimum(60);
	lpm->setMaximum(360);
	lpm->setSingleStep(10);
	lpm->setValue(config.readNumEntry("/hamfax/fax/LPM"));
	connect(lpm,SIGNAL(valueChanged(int)),SLOT(setLpm(int)));
	lpm->setToolTip(tr("lines per minute"));

	faxTool->addSeparator();

	faxTool->addWidget(new QLabel(tr("phasing lines")));

	QSpinBox* phaseLines = new QSpinBox();
	faxTool->addWidget(phaseLines);
	phaseLines->setMinimum(0);
	phaseLines->setMaximum(50);
	phaseLines->setSingleStep(1);
	phaseLines->setValue(config.readNumEntry("/hamfax/phasing/lines"));
	connect(phaseLines,SIGNAL(valueChanged(int)),SLOT(setPhaseLines(int)));
	phaseLines->setToolTip(tr("phasing lines mark the beginning\n"
				  "of a line and the speed (lpm)"));

	invertPhase = new QComboBox();
	faxTool->addWidget(invertPhase);
	invertPhase->addItem(tr("normal"));
	invertPhase->addItem(tr("inverted"));
	invertPhase->setCurrentIndex(config.readBoolEntry("/hamfax/phasing/invert")
				    ? 1 : 0);
	connect(invertPhase,SIGNAL(activated(int)),SLOT(setPhaseInvert(int)));
	invertPhase->setToolTip(tr("normal means 2.5% white, 95% black\n"
				   "and again 2.5% white"));

	faxTool->addSeparator();

	colorBox = new QComboBox();
	faxTool->addWidget(colorBox);
	colorBox->addItem(tr("mono"));
	colorBox->addItem(tr("color"));
	colorBox->setCurrentIndex(config.readBoolEntry("/hamfax/fax/color")?1:0);
	connect(colorBox,SIGNAL(activated(int)),SLOT(setColor(int)));
	colorBox->setToolTip(tr("In color mode each line\n"
				"is split in three lines:\n"
				"red, green and blue."));
}

void FaxWindow::createStatusBar()
{
	statusBar()->setSizeGripEnabled(false);
	sizeText = new QLabel();
	statusBar()->addPermanentWidget(sizeText);
	iocText = new QLabel();
	statusBar()->addPermanentWidget(iocText);
	iocText->setToolTip(tr("Index Of Cooperation:\n"
			       "image width in pixels divided by PI"));
}

void FaxWindow::endTransmission(void)
{
	switch(interface) {
	case FILE:
		file->end();
		disconnect(file, SIGNAL(next(int)),
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
	switch(QMessageBox::information(this, windowTitle(),
					tr("Really exit?"),
					tr("&Exit"),tr("&Don't Exit"))) {
	case 0:
		Config::instance().setValue("GUI/geometry", saveGeometry());
		Config::instance().setValue("GUI/windowState", saveState());
		close->accept();
		break;
	case 1:
		close->ignore();
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
	slantAction->setEnabled(b);
	colorDrawAction->setEnabled(b);
	monoDrawAction->setEnabled(b);
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
	const QList<QByteArray>& formats = QImageReader::supportedImageFormats();
	QStringList str_formats;

	for (int i = 0; i < formats.size(); i++)
		str_formats.append(formats.at(i));

	QString filter = "*." + str_formats.join(" *.").toLower();
	QString name = QFileDialog::getOpenFileName(this, windowTitle(), ".",
						    filter);

	if(!name.isEmpty()) {
		faxImage->load(name);
	}
}

void FaxWindow::save(void)
{
	const QList <QByteArray>& formats = QImageWriter::supportedImageFormats();
	QStringList str_formats;

	for (int i = 0; i< formats.size(); i++)
		str_formats.append(formats.at(i));

	QString filter = "*." + str_formats.join(" *.").toLower();
	QString name = QFileDialog::getSaveFileName(this, windowTitle(), ".",
						    filter);

	if(!name.isEmpty()) {
		bool result = faxImage->save(name);
		if (result == false)
			QMessageBox::warning(this, windowTitle(),
				tr("Could not save to file '%1'.").arg(name),
				QMessageBox::Ok, Qt::NoButton);
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

void FaxWindow::initTransmitCommon(int interface, int sampleRate)
{
	this->interface = interface;
	faxTransmitter->start(sampleRate);
	faxModulator->init(sampleRate);
	transmitDialog->start();
	disableControls();
}

void FaxWindow::initTransmitFile()
{
	try {
		QString	fileName;
		fileName = QFileDialog::getSaveFileName(this, windowTitle(),
							".", "*.au");
		if(fileName.isEmpty())
			return;

		int sampleRate = file->startOutput(fileName);

		connect(file,SIGNAL(next(int)), faxTransmitter,SLOT(doNext(int)));
		connect(faxTransmitter,	SIGNAL(data(double*, int)),
			faxModulator, SLOT(modulate(double*, int)));
		connect(faxModulator, SIGNAL(data(short*, int)),
			file, SLOT(write(short*, int)));

		initTransmitCommon(FILE, sampleRate);
	} catch (Error e) {
		QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::initTransmitDsp()
{
	try {
		int sampleRate=sound->startOutput();
		connect(sound,SIGNAL(spaceLeft(int)),
			faxTransmitter,SLOT(doNext(int)));
		connect(faxTransmitter,	SIGNAL(data(double*, int)),
			faxModulator, SLOT(modulate(double*, int)));
		connect(faxModulator, SIGNAL(data(short*, int)),
			sound, SLOT(write(short*, int)));

		initTransmitCommon(DSP, sampleRate);
	} catch (Error e) {
		QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::initTransmitPtc()
{
	try {
		int sampleRate=ptc->startOutput();
		connect(ptc,SIGNAL(spaceLeft(int)),
			faxTransmitter,SLOT(doNext(int)));
		connect(faxTransmitter,SIGNAL(data(double*, int)),
			ptc,SLOT(transmit(double*, int)));

		initTransmitCommon(SCSPTC, sampleRate);
	} catch (Error e) {
		QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::initReceptionCommon(int interface, int sampleRate)
{
	this->interface = interface;
	faxReceiver->init(sampleRate);
	receiveDialog->aptStart();
	disableControls();
	faxDemodulator->init(sampleRate);
}

void FaxWindow::initReceptionFile()
{
	try {
		QString fileName;
		fileName = QFileDialog::getOpenFileName(this, windowTitle(),
							".", "*.au");
		if(fileName.isEmpty())
			return;
		int sampleRate=file->startInput(fileName);
		connect(file, SIGNAL(data(short*,int)),
			faxDemodulator, SLOT(newSamples(short*,int)));
		connect(faxDemodulator, SIGNAL(data(int*,int)),
			faxReceiver, SLOT(decode(int*,int)));

		initReceptionCommon(FILE, sampleRate);
	} catch (Error e) {
                QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::initReceptionDsp()
{
	try {
		int sampleRate=sound->startInput();
		connect(sound,SIGNAL(data(short*,int)),
			faxDemodulator, SLOT(newSamples(short*,int)));
		connect(faxDemodulator, SIGNAL(data(int*,int)),
			faxReceiver, SLOT(decode(int*,int)));

		initReceptionCommon(DSP, sampleRate);
	} catch (Error e) {
                QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::initReceptionPtc()
{
        try {
		int sampleRate=ptc->startInput();
		connect(ptc,SIGNAL(data(int*,int)),
			faxReceiver, SLOT(decode(int*, int)));

		initReceptionCommon(SCSPTC, sampleRate);
	} catch(Error e) {
		QMessageBox::warning(this, windowTitle(), e.getText());
	}
}

void FaxWindow::adjustIOC(void)
{
	bool ok;
	int iocNew = QInputDialog::getInteger(this, windowTitle(),
					      tr("Please enter IOC"), ioc,
					      204, 576, 1, &ok);
	if(ok) {
		faxReceiver->correctWidth(M_PI*iocNew);
	}
}

void FaxWindow::scaleToIOC(void)
{
	bool ok;
	int newIOC = QInputDialog::getInteger(this, windowTitle(),
					      tr("Please enter IOC"), ioc,
					      204, 576, 1, &ok);
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
	colorBox->setCurrentIndex(1);
	faxReceiver->correctLPM(0);
}

void FaxWindow::redrawMono(void)
{
	Config::instance().writeEntry("/hamfax/fax/color",false);
	colorBox->setCurrentIndex(0);
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
		QApplication::setFont(f);
		Config::instance().writeEntry("/hamfax/GUI/font",f.toString());
	}
}

void FaxWindow::changePTT(void)
{
	bool b = !pttAction->isChecked();
	pttAction->setChecked(b);
	Config::instance().writeEntry("/hamfax/PTT/use", b);
}

void FaxWindow::changeScroll(void)
{
	bool b = !scrollAction->isChecked();
	scrollAction->setChecked(b);
	Config::instance().writeEntry("/hamfax/GUI/autoScroll",b);
	faxImage->setAutoScroll(b);
}

void FaxWindow::changeToolTip(void)
{
	bool b = !toolTipAction->isChecked();
	toolTipAction->setChecked(b);
	Config::instance().writeEntry("/hamfax/GUI/toolTips",b);
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
		this, windowTitle(),
		tr("hamfax is a Qt application for transmitting and receiving"
		   "\nham radio facsimiles and weather broadcasts.\n\n"
		   "Author: Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>"
		   "\n\nThe demodulator is taken from ACfax"
		   " by Andreas Czechanowski, DL4SDC"
		   "\n\nLicense: GNU General Public License"
		   "\nVersion: %1").arg(windowTitle()));
}

void FaxWindow::aboutQT(void)
{
	QMessageBox::aboutQt(this, windowTitle());
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
	filter->setCurrentIndex(n);
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
