// HamFax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001 Christof Schmitt, DH1CS <christof.schmitt@gmx.de>
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

#include "File.hpp"
#include "Error.hpp"

File::File(QObject* parent)
	: QObject(parent), sampleRate(8000)
{
	afSetErrorHandler(0);
	timer=new QTimer(this);
}

File::~File(void)
{
	if(aFile!=0) {
		afCloseFile(aFile);
		aFile=0;
	}
}

void File::startOutput(const QString& fileName)
{
	try {
		AFfilesetup fs;
		if((fs=afNewFileSetup())==AF_NULL_FILESETUP) {
			throw Error(tr("could not allocate AFfilesetup"));
		}
		afInitFileFormat(fs,AF_FILE_NEXTSND);
		afInitSampleFormat(fs,AF_DEFAULT_TRACK,AF_SAMPFMT_TWOSCOMP,16);
		afInitByteOrder(fs,AF_DEFAULT_TRACK,AF_BYTEORDER_BIGENDIAN);
		afInitChannels(fs,AF_DEFAULT_TRACK,1);
		afInitRate(fs,AF_DEFAULT_TRACK,sampleRate=8000);
		
		if((aFile=afOpenFile(fileName,"w",fs))==AF_NULL_FILEHANDLE) {
			aFile=0;
			throw Error(tr("could not open file"));
		}
		afFreeFileSetup(fs);
		timer->start(0);
		connect(timer,SIGNAL(timeout()),this,SLOT(timerSignal()));
		emit newSampleRate(sampleRate);
	} catch(Error) {
		end();
		throw;
	}
}

void File::startInput(const QString& fileName)
{
	try {
		AFfilesetup fs=0;
		if((aFile=afOpenFile(fileName,"r",fs))==AF_NULL_FILEHANDLE) {
			throw Error(tr("could not open file"));
		}
		if(afGetFrameSize(aFile,AF_DEFAULT_TRACK,0)!=2) {
			throw Error(tr("samples size not 16 bit"));
		}
		emit newSampleRate(afGetRate(aFile,AF_DEFAULT_TRACK));
		timer->start(0);
		connect(timer,SIGNAL(timeout()),this,SLOT(read()));
	} catch(Error) {
		end();
		throw;
	}
}

void File::end(void)
{
	timer->stop();
	disconnect(timer,SIGNAL(timeout()),this,SLOT(timerSignal()));
	if(aFile!=0) {
		afCloseFile(aFile);
		aFile=0;
		emit deviceClosed();
	}
}

void File::write(short* samples, int number)
{
	if(aFile!=0) {
		if((afWriteFrames(aFile,AF_DEFAULT_TRACK,
				  samples,number))==AF_BAD_WRITE) {
			end();
			}
	}
}

void File::read(void)
{
	int n=512;
	short buffer[n];
	n=afReadFrames(aFile,AF_DEFAULT_TRACK,buffer,n);
	emit data(buffer,n);
}

void File::timerSignal(void)
{
	emit next(512);
}
