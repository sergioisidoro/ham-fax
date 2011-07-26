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

#include "Sound.hpp"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <qtimer.h>
#include <qthread.h>
#include <cstring>
#include "Config.hpp"
#include "Error.hpp"

Sound::Sound(QObject* parent)
	: QObject(parent), sampleRate(8000), 
	  use_alsa(1),
#ifdef USE_ALSA
	  pcm(NULL), handler(NULL), frames(512), framesize(sizeof(short)),
	  buffer(NULL), pollfds(NULL), notifiers(NULL),
#endif
	  dsp(-1), notifier(0), rateF(1)
{
	char *pszSr = getenv("DEF_RATE");
	int nsr; 

	if (pszSr && sscanf (pszSr, "%d", &nsr) == 1) {

		sampleRate = nsr;

		rateF = sampleRate / 8000;

		printf("New sample rate: %d rate: %d\n", sampleRate, rateF);
	}
}

Sound::~Sound(void)
{
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

void Sound::setupNotifiers(const char *method)
{
	count_fds = snd_pcm_poll_descriptors_count(pcm);
	if (count_fds <= 0)
		throw Error(tr("ALSA: invalid poll descriptors count: %1").
			    arg(snd_strerror(count_fds)));

	pollfds = new pollfd[count_fds];
	int rc = snd_pcm_poll_descriptors(pcm, pollfds, count_fds);
	if (rc < 0)
		throw Error(tr("ALSA: Unable to obtain poll descriptors: %1").
			    arg(snd_strerror(rc)));

	int count_ns = 0;

	for (int i = 0; i < count_fds; i++) {
		if (pollfds[i].events & POLLIN)
			count_ns++;
		if (pollfds[i].events & POLLOUT)
			count_ns++;
	}

	// Add one to mark the end with a NULL pointer.
	notifiers = new QSocketNotifier*[count_ns + 1];
	std::memset(notifiers, 0, (count_ns + 1) * sizeof(QSocketNotifier*));
	QSocketNotifier **notifier = notifiers;

	for (int i = 0; i < count_fds; i++) {
		if (pollfds[i].events & POLLIN) {
			*notifier = new QSocketNotifier(pollfds[i].fd,
							QSocketNotifier::Read,
							this);
		}
		if (pollfds[i].events & POLLOUT) {
			*notifier = new QSocketNotifier(pollfds[i].fd,
							QSocketNotifier::Write,
							this);
		}
		connect(*notifier, SIGNAL(activated(int)), this, method);
	}
}

void Sound::deleteNotifiers(void)
{
	QSocketNotifier **notifier = notifiers;

	while (*notifier) {
		(*notifier)->setEnabled(false);
		(*notifier)->disconnect();
		delete *notifier;
		notifier++;
	}

	delete[] notifiers;
	notifiers = 0;
}

#endif

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

	        int rc = snd_pcm_open(&pcm, devDSPName.toAscii(),
				      SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
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

		setupNotifiers(SLOT(checkSpace(int)));
		snd_pcm_start(pcm);
	     } else {
#endif /* USE_ALSA */
		
		dsp = open(devDSPName.toAscii(), O_WRONLY | O_NONBLOCK);
		if(dsp == -1) {
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

	        int rc = snd_pcm_open(&pcm, devDSPName.toAscii(),
				      SND_PCM_STREAM_CAPTURE,
				      SND_PCM_ASYNC | SND_PCM_NONBLOCK);
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

		buffer=(short *)malloc(frames * framesize);
		
		setupNotifiers(SLOT(readALSA(int)));
		snd_pcm_start(pcm);
		
	     } else {
#endif /* USE_ALSA */
		dsp = open(devDSPName.toAscii(), O_RDONLY | O_NONBLOCK);
		if (dsp == -1) {
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

		// trigger the file descriptor reads
		read (dsp);

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

#ifdef USE_ALSA
	if (notifiers)
		deleteNotifiers();
#endif
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
		}
#endif /* USE_ALSA */
	} catch(Error) {
		close();
	}
}

void Sound::read(int fd)
{
    #define RX_BUF (BUFSIZ*8) 
	short buffer [RX_BUF];
    short usbuf  [RX_BUF/2];

    int n=::read (fd, buffer, sizeof(buffer));

    if ( n > 0 ) {

        #if 0
        int i;
        int ns = n / 2;  // # of shorts !!!
        int j;

        //for (i=0; i < n; ++i) {
        //    printf("i=%d, value=%d ", i, (int)buffer[j]);
        //}
        

        // copy the even short only !!!
        for (i = 0, j=0; i < ns; i++) {
            if ( (i % 2) == 0 ) {
                usbuf[j] = buffer [i];
                //printf("i=%d, j=%d, value=%d ", i, j, (int)usbuf[j]);
                j++;
            }
        }
        //printf("READ: %d\n", i);
        emit data ( usbuf, j);
        #else
        emit data ( buffer, n / sizeof(short));
        #endif
    }
}

#ifdef USE_ALSA
void Sound::readALSA(int fd)
{
	int n;
	unsigned short revents;

	snd_pcm_poll_descriptors_revents(pcm, pollfds, count_fds, &revents);
	if (revents & POLLERR) {
		snd_pcm_recover(pcm, -EPIPE, 0);
		return;
	}

	if (!(revents & POLLIN))
		return;

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
}
#else /* USE_ALSA */
// empty stub since it cannot be ifdef'ed in the hpp file
void Sound::readALSA(int fd) { }
#endif /* USE_ALSA */

void Sound::checkSpace(int fd)
{
#ifdef USE_ALSA
	if (pcm) {
		unsigned short revents;
		snd_pcm_poll_descriptors_revents(pcm, pollfds, count_fds,
						 &revents);

		if (revents & POLLERR)
			snd_pcm_recover(pcm, -EPIPE, 0);
		else if (revents & POLLOUT)
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

	if (pollfds) {
		delete[] pollfds;
		pollfds = 0;
	}

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
