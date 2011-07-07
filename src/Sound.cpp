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

#include "Sound.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>
#include <qtimer.h>
#include <qthread.h>
#include "Config.hpp"
#include "Error.hpp"

#ifdef QT_THREAD_SUPPORT
// If QT was built with thread support, we can do things
// a little differently (better)
// *** This is untested as my QT does not have threads! ***
//#define HF_QT_THREAD
#endif

#ifdef HF_QT_THREAD
#include <qwaitcondition.h>
class TransferThread: public QThread {
public:
	virtual void run();
	Sound   *sound;
	int     quit;
	int     mode;
	QWaitCondition have_data;
};

#define XM_READ  1
#define XM_WRITE 2
void TransferThread::run()
{
	quit=0;
	while (!quit) {
	    have_data.wait();
	    if (mode==XM_WRITE) {
		sound->checkSpace(-1);
	    } else {
	       while (!quit && sound->readALSAdata());
	    }
	}
}

#else

#include <sys/socket.h>

#endif


Sound::Sound(QObject* parent)
	: QObject(parent), sampleRate(8000), 
	  use_alsa(1),
#ifdef USE_ALSA
	  pcm(NULL), handler(NULL), frames(512), framesize(sizeof(short)),
	  buffer(NULL),
#endif
#ifdef HF_QT_THREAD
	  xfer_thread(NULL),
#endif
	  dsp(-1), notifier(0)
{
}

Sound::~Sound(void)
{
#ifdef HF_QT_THREAD
	if (xfer_thread) {
	   xfer_thread->quit=1;
	   xfer_thread->have_data.wake();
	   xfer_thread.wait();
	   delete xfer_thread;
	   xfer_thread=NULL;
	}
#endif
	if(dsp!=-1) {
		::close(dsp);
	}
#ifdef USE_ALSA
	if (pcm) {
		snd_pcm_drain(pcm);
		snd_pcm_close(pcm);
		pcm=NULL;
	}
	if (buffer) {
	   free(buffer);
	   buffer=NULL;
	}
#endif
}


#ifdef USE_ALSA
/*
   A couple of functions to turn ALSA callbacks into:
   No QThreads: socket activity which is picked up by a QSocketNotifier
                and triggers a read/write of audio data.
   With QThreads: wakeup of the TransferThread to do the read/write
*/
void Sound::ALSA_write_callback(snd_async_handler_t *handler)
{
  Sound *sound=(Sound *)snd_async_handler_get_callback_private(handler);
#ifdef HF_QT_THREAD
  sound->xfer_thread->have_data.wake();
#else
  char a=1;
  ::write(sound->callbackSocket[0], &a, sizeof(a));
#endif
}

void Sound::ALSA_read_callback(snd_async_handler_t *handler)
{
  Sound *sound=(Sound *)snd_async_handler_get_callback_private(handler);
#ifdef HF_QT_THREAD
  sound->xfer_thread->have_data.wake();
#else
  char a=1;
  ::write(sound->callbackSocket[0], &a, sizeof(a));
#endif
}
#endif /* USE_ALSA */

int Sound::startOutput(void)
{
	try {
	     unsigned int speed=sampleRate;
	     QString devDSPName=Config::instance().
			readEntry("/hamfax/sound/device");

	     if (devDSPName.startsWith("ALSA:")) {
	        use_alsa=1;
	        devDSPName=devDSPName.mid(5);
	     }
	     else {
	        use_alsa=0;
	     }

#ifdef USE_ALSA
	     if (use_alsa) {
	        snd_pcm_hw_params_t *hwparams=NULL;
	        snd_pcm_sw_params_t *swparams=NULL;
		int dir=0; // 0=match speed, -1 nearest below, +1 nearest above

	        int rc=snd_pcm_open(&pcm, devDSPName, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	        if (rc<0) throw Error(tr("could not open ALSA sound device"));
	        
	        // allocate parameters and fill with defaults
	        snd_pcm_hw_params_alloca(&hwparams);
	        snd_pcm_hw_params_any(pcm, hwparams);

		/* Interleaved mode */
		snd_pcm_hw_params_set_access(pcm, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);

		/* Signed 16-bit little-endian format */
		snd_pcm_hw_params_set_format(pcm, hwparams,
			SND_PCM_FORMAT_S16_LE);

		/* One channel (mono) */
		snd_pcm_hw_params_set_channels(pcm, hwparams, 1);

		/* sampling rate */
		snd_pcm_hw_params_set_rate_near(pcm, hwparams,
			&speed, &dir);
	        if(rc<0 || speed<(sampleRate*0.99) || speed>(sampleRate*1.01)) {
	        	throw Error(tr("could not set sample rate"));
		}

		/* Write the parameters to the driver */
		rc = snd_pcm_hw_params(pcm, hwparams);
		if (rc < 0) {
		   throw Error(tr(snd_strerror(rc)));
		}

		// setup software parameters
		snd_pcm_sw_params_malloc(&swparams);
		snd_pcm_sw_params_current(pcm, swparams);
		// - only call back when there's a decent chunk of data
		snd_pcm_sw_params_set_avail_min(pcm, swparams, frames);
		snd_pcm_sw_params(pcm, swparams);

		//snd_pcm_hw_params_free(hwparams);
		//snd_pcm_sw_params_free(swparams);

		//buffer=(short *)malloc(frames * framesize);

#ifdef HF_QT_THREAD
		// set up a thread waiting for data
		if (!xfer_thread) xfer_thread = new TransferHandler();
		xfer_thread->sound=this;
		xfer_thread->mode=XM_WRITE;
		xfer_thread->start(QThread::HighPriority);
#else
		// set up a unix socket with a notifier on it
		if (::socketpair(AF_UNIX, SOCK_STREAM, 0, callbackSocket)) {
		   // couldn't create socket pair...
		   }
		fcntl(callbackSocket[0], F_SETFL, O_NONBLOCK);
		fcntl(callbackSocket[1], F_SETFL, O_NONBLOCK);

		notifier=new QSocketNotifier(callbackSocket[1],QSocketNotifier::Write,this);
		connect(notifier,SIGNAL(activated(int)),
			this,SLOT(checkSpace(int)));
#endif
		
		snd_async_add_pcm_handler(&handler, pcm, ALSA_write_callback, this);
		//snd_pcm_start(pcm);
	     } else {
#endif /* USE_ALSA */
		
		if((dsp=open(devDSPName,O_WRONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_NE;
		if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_NE) {
			throw Error(
				tr("could not set audio format to S16_NE"));
		}
		int channels=1;
		if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		ioctl(dsp,SNDCTL_DSP_SPEED,&speed);
		if(speed<sampleRate*0.99 || speed>sampleRate*1.01) {
			throw Error(tr("could not set sample rate"));
		}
		notifier=new QSocketNotifier(dsp,QSocketNotifier::Write,this);
		connect(notifier,SIGNAL(activated(int)),
			this,SLOT(checkSpace(int)));
		ptt.set();
#ifdef USE_ALSA
	     }
#endif /* USE_ALSA */
	} catch(Error) {
		close();
		throw;
	}
	return sampleRate;
}


int Sound::startInput(void)
{
	try {
	     unsigned int speed=sampleRate;
	     QString devDSPName=Config::instance().
			readEntry("/hamfax/sound/device");

	     if (devDSPName.startsWith("ALSA:")) {
	        use_alsa=1;
	        devDSPName=devDSPName.mid(5);
	     }
	     else {
	        use_alsa=0;
	     }
			
#ifdef USE_ALSA
	     if (use_alsa) {
	        snd_pcm_hw_params_t *hwparams=NULL;
	        snd_pcm_sw_params_t *swparams=NULL;
		int dir=0; // 0=match speed, -1 nearest below, +1 nearest above

	        int rc=snd_pcm_open(&pcm, devDSPName, SND_PCM_STREAM_CAPTURE, SND_PCM_ASYNC | SND_PCM_NONBLOCK);
	        if (rc<0) throw Error(tr("could not open ALSA:default"));
	        
	        // allocate parameters and fill with defaults
	        snd_pcm_hw_params_alloca(&hwparams);
	        snd_pcm_hw_params_any(pcm, hwparams);

		/* Interleaved mode */
		snd_pcm_hw_params_set_access(pcm, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);
		
		/* Signed 16-bit little-endian format */
		snd_pcm_hw_params_set_format(pcm, hwparams,
			SND_PCM_FORMAT_S16_LE);

		/* One channel (mono) */
		snd_pcm_hw_params_set_channels(pcm, hwparams, 1);

		/* sampling rate - MUST be what we want */
		dir=0;
		rc=snd_pcm_hw_params_set_rate_near(pcm, hwparams,
			&speed, &dir);
	        if(rc<0 || speed<(sampleRate*0.99) || speed>(sampleRate*1.01)) {
	        	throw Error(tr("could not set sample rate"));
		}

		/* bigger buffers */
		snd_pcm_uframes_t bufsize;
		snd_pcm_hw_params_get_buffer_size_max(hwparams, &bufsize);
		if (bufsize>(frames*10)) bufsize=frames*10;
		snd_pcm_hw_params_set_buffer_size(pcm, hwparams, bufsize);

		/* Write the parameters to the driver */
		rc = snd_pcm_hw_params(pcm, hwparams);
		if (rc < 0) {
		   fprintf(stderr, "write parameters failed: %s\n", snd_strerror(rc));
		   throw Error(tr(snd_strerror(rc)));
		}

		// setup software parameters
		snd_pcm_sw_params_malloc(&swparams);
		snd_pcm_sw_params_current(pcm, swparams);
		// - only call back when there's a decent chunk of data
		snd_pcm_sw_params_set_avail_min(pcm, swparams, frames);
		snd_pcm_sw_params(pcm, swparams);
		
//		doing this seems to crash things - looks like
//		the parameter blocks are supposed to exist for
//		the life of the pcm.
//		snd_pcm_hw_params_free(hwparams);
//		snd_pcm_sw_params_free(swparams);

		rc=snd_async_add_pcm_handler(&handler, pcm, ALSA_read_callback, this);
		if (rc<0) {
		   fprintf(stderr, "add_handler failed: %s\n", snd_strerror(rc));
		   throw Error(tr(snd_strerror(rc)));
		}

		buffer=(short *)malloc(frames * framesize);

#ifdef HF_QT_THREAD
		// set up a thread waiting for data
		if (!xfer_thread) xfer_thread = new TransferHandler();
		xfer_thread->sound=this;
		xfer_thread->mode=XM_READ;
		xfer_thread->start(QThread::HighPriority);
#else
		// set up a unix socket with a notifier on it
		if (::socketpair(AF_UNIX, SOCK_STREAM, 0, callbackSocket)) {
		   // couldn't create socket pair...
		   }
		fcntl(callbackSocket[1], F_SETFL, O_NONBLOCK);
		
		notifier=new QSocketNotifier(callbackSocket[1],QSocketNotifier::Read,this);
		connect(notifier,SIGNAL(activated(int)),
			this,SLOT(readALSA(int)));
#endif
		
		snd_pcm_start(pcm);
		
	     } else {
#endif /* USE_ALSA */
		if((dsp=open(devDSPName,O_RDONLY|O_NONBLOCK))==-1) {
			throw Error(tr("could not open dsp device"));
		}
		int format=AFMT_S16_LE;
		if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1
		   ||format!=AFMT_S16_LE) {
			throw Error(
				tr("could not set audio format S16_LE"));
		}
		int channels=1;
		if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1
		   ||channels!=1) {
			throw Error(tr("could not set mono mode"));
		}
		ioctl(dsp,SNDCTL_DSP_SPEED,&speed);
	     
	        if(speed<(sampleRate*0.99) || speed>(sampleRate*1.01)) {
	        	throw Error(tr("could not set sample rate"));
		}
		notifier=new QSocketNotifier(dsp,QSocketNotifier::Read,this);
		connect(notifier,SIGNAL(activated(int)),SLOT(read(int)));
#ifdef USE_ALSA
	     }
#endif /* USE_ALSA */
	} catch(Error) {
		close();
		throw;
	}
	return sampleRate;
}

void Sound::end(void)
{
#ifdef USE_ALSA
	if (pcm) snd_pcm_drain(pcm);
#endif /* USE_ALSA */
	
	if(notifier) {
		notifier->setEnabled(false);
		if(notifier->type()==QSocketNotifier::Read) {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(read(int)));
			close();
		} else {
			disconnect(notifier,SIGNAL(activated(int)),
				   this,SLOT(checkSpace(int)));
			int i=2;
			if (dsp!=-1) ioctl(dsp,SNDCTL_DSP_GETODELAY,&i);
			QTimer::singleShot(1000*i
					   /sampleRate/sizeof(short),
					   this,SLOT(close()));
		}
		delete notifier;
		notifier=NULL;
	} else {
		QTimer::singleShot(1000,this,SLOT(close()));
	}
}

void Sound::write(short* samples, int number)
{
	try {
		if(dsp!=-1) {
			notifier->setEnabled(false);
			if((::write(dsp,samples, number*sizeof(short)))
			   !=static_cast<int>(number*sizeof(short))) {
				throw Error();
			}
			notifier->setEnabled(true);
		}
#ifdef USE_ALSA
		if (pcm) {
#ifndef HF_QT_THREAD
			notifier->setEnabled(false);
#endif
			int rc=snd_pcm_writei(pcm,samples, number);
			if (rc == -EPIPE) {
			   // underrun occurred
			   snd_pcm_recover(pcm,rc,1);
			} else if (rc < 0) {
			   // other error (recover or print it)
			   if (snd_pcm_recover(pcm, rc, 1)) {
			      fprintf(stderr, "ALSA write error: %s\n", snd_strerror(rc));
			      snd_pcm_prepare(pcm);
			      }
			} else if (rc != number) {
				throw Error();
			}
#ifndef HF_QT_THREAD
			notifier->setEnabled(true);
#endif
		}
#endif /* USE_ALSA */
	} catch(Error) {
		close();
	}
}

void Sound::read(int fd)
{
	const int max=256;
	short buffer[max];
	int n=::read(fd,buffer,max*sizeof(short))/sizeof(short);
	if(n>0 && n<=max) {
		emit data(buffer,n);
	}
}

#ifdef USE_ALSA
// stub for when using non-threaded approach
void Sound::readALSA(int fd)
{
  char tmp;
  ::read(fd, &tmp, sizeof(tmp));
  //while (readALSAdata());
  readALSAdata();
}
#else /* USE_ALSA */
// empty stub since it cannot be ifdef'ed in the hpp file
void Sound::readALSA(int fd) { }
#endif /* USE_ALSA */

#ifdef USE_ALSA
bool Sound::readALSAdata()
{
	int n;

	if (!pcm) return FALSE;
	n=snd_pcm_readi(pcm, buffer, frames);
	if (n == -EPIPE) {
	   // overrun
	   fprintf(stderr, "ALSA overrun\n");
	   snd_pcm_recover(pcm,n,0);
	   snd_pcm_start(pcm);
	} else if (n == -EAGAIN) {
	   // no data available
	} else if (n < 0) {
	   // other error - recover or print
	   if (snd_pcm_recover(pcm,n,0)) {
	      fprintf(stderr, "ALSA Read error:%s\n",snd_strerror(n));
	      snd_pcm_prepare(pcm);
	      }
	   snd_pcm_start(pcm);
	}
//	else { fprintf(stderr,"ALSA read ok %d frames\n",n); }
	  
	if(n>0 && n<=(int)frames) {
		emit data(buffer,n);
	}
	return (n>0 && n<=(int)frames);;
}
#endif /* USE_ALSA */

void Sound::checkSpace(int fd)
{
#ifdef USE_ALSA
	if (pcm) {
#ifndef HF_QT_THREAD
	  char tmp;
	  ::read(callbackSocket[1], &tmp, sizeof(tmp));
#endif
	  emit spaceLeft(snd_pcm_avail_update(pcm));
	} else 
#endif /* USE_ALSA */
	if (dsp!=-1) {
	  audio_buf_info info;
	  ioctl(fd,SNDCTL_DSP_GETOSPACE,&info);
	  emit spaceLeft(info.bytes/sizeof(short));
	} else {
	  emit spaceLeft(0);
	}
}

void Sound::close(void)
{
	if (dsp!=-1) {
	   ioctl(dsp,SNDCTL_DSP_RESET);
	   ::close(dsp);
	   dsp=-1;
	}
#ifdef USE_ALSA
	if (pcm) {
	   snd_pcm_drop(pcm);
	   if (handler) {
		//snd_async_del_handler(handler);
		handler=NULL;
	   }
	   snd_pcm_close(pcm);
	   pcm=NULL;
	}
#ifdef HF_QT_THREAD
	if (xfer_thread) {
		xfer_thread->quit=1;
		xfer_thread->have_data.wake();
	}
#else
	if (callbackSocket[0]!=-1) ::close(callbackSocket[0]);
	if (callbackSocket[1]!=-1) ::close(callbackSocket[1]);
	callbackSocket[0]=-1;
	callbackSocket[1]=-1;
#endif
	
	if (buffer) {
	   free(buffer);
	   buffer=NULL;
	}
#endif /* USE_ALSA */

	emit deviceClosed();
	ptt.release();
}

void Sound::closeNow(void)
{
	fprintf(stderr,"Sound::closeNow\n");
	if(dsp!=-1) {
		notifier->setEnabled(false);
		delete notifier;
		notifier=NULL;
	}
	if (dsp!=-1) {
		close();
		return;
	}
#ifdef USE_ALSA
	if (pcm)
		close();
#endif /* USE_ALSA */
}
