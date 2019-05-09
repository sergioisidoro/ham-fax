// hamfax -- an application for sending and receiving amateur radio facsimiles
// Copyright (C) 2001, 2002, 2012
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

#include "File.hpp"
#include "Error.hpp"

static const char *last_error = NULL;

static void audiofile_error(long l, const char *c)
{
	last_error = c;
}

File::File(QObject* parent)
	: QObject(parent), aFile(0)
{
	afSetErrorHandler(audiofile_error);
	timer=new QTimer(this);
}

File::~File(void)
{
	end();
}

int File::startOutput(const QString& fileName)
{
	try {
		AFfilesetup fs;
		if((fs=afNewFileSetup())==AF_NULL_FILESETUP) {
			throw Error(tr("could not allocate AFfilesetup: %1").
				    arg(last_error));
		}
		afInitFileFormat(fs,AF_FILE_NEXTSND);
		afInitSampleFormat(fs,AF_DEFAULT_TRACK,AF_SAMPFMT_TWOSCOMP,16);
		afInitByteOrder(fs,AF_DEFAULT_TRACK,AF_BYTEORDER_BIGENDIAN);
		afInitChannels(fs,AF_DEFAULT_TRACK,1);
		afInitRate(fs,AF_DEFAULT_TRACK,8000);
		
		aFile = afOpenFile(fileName.toLatin1(), "w", fs);
		if(aFile == AF_NULL_FILEHANDLE) {
			aFile=0;
			throw Error(tr("could not open file: %1").
				    arg(last_error));
		}
		afFreeFileSetup(fs);
		timer->start(0);
		connect(timer,SIGNAL(timeout()),this,SLOT(timerSignal()));
	} catch(Error) {
		end();
		throw;
	}
	return 8000;
}

int File::startInput(const QString& fileName)
{
	try {
		AFfilesetup fs=0;
		aFile = afOpenFile(fileName.toLatin1(), "r", fs);
		if(aFile == AF_NULL_FILEHANDLE) {
			throw Error(tr("could not open file: %1").
				    arg(last_error));
		}
		if(afGetFrameSize(aFile,AF_DEFAULT_TRACK,0)!=2) {
			throw Error(tr("samples size not 16 bit"));
		}
		if(afGetRate(aFile,AF_DEFAULT_TRACK)!=8000) {
			throw Error(tr("sample rate is not 8000 Hz"));
		}
		timer->start(0);
		connect(timer,SIGNAL(timeout()),this,SLOT(read()));
	} catch(Error) {
		end();
		throw;
	}
	return 8000;
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
		if((afWriteFrames(aFile,AF_DEFAULT_TRACK,samples,number))
		   !=blockSize) {
			end();
			}
	}
}

void File::read(void)
{
	int n=blockSize;
	short buffer[n];
	n=afReadFrames(aFile,AF_DEFAULT_TRACK,buffer,n);
	emit data(buffer,n);
}

void File::timerSignal(void)
{
	emit next(blockSize);
}
