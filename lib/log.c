/* hamfax -- an application for sending and receiving amateur radio facsimiles
 * Copyright (C) 2011
 * Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "config.h"

#ifdef HAVE_LIBHAMLIB
#include <hamlib/rig.h>
#endif

static FILE *log_file;

#ifdef HAVE_LIBHAMLIB
/**
 * The default setting for hamlib is logging to stderr. Adjust it to our log,
 * or use /dev/null to redirect the hamlib log.
 */
static void set_hamlib_log(FILE *file)
{
	if (log_file) {
		rig_set_debug_file(log_file);

	} else {
		FILE *dev_null;

		dev_null = fopen("/dev/null", "w");
		if (dev_null)
			rig_set_debug_file(dev_null);
	}
}
#else
static inline void set_hamlib_log(FILE *file) { }
#endif

int log_open(const char *file)
{
	log_file = fopen(file, "a");
	if (!log_file)
		return errno;

	set_hamlib_log(log_file);
	return 0;
}

int log_open_argv(int argc, char **argv)
{
	int i;
	int open = 0;
	char *filename = NULL;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-l") == 0) {
			open = 1;
			continue;
		}

		if (open && argv[i][0] != '-') {
			filename = argv[i];
			break;
		}
	}

	if (open && filename)
		return log_open(filename);

	if (open && !filename)
		log_file = stderr;

	set_hamlib_log(log_file);

	return 0;
}

int log_close()
{
	if (log_file && log_file != stderr)
		fclose(log_file);

	log_file = NULL;
}

void log_debug_(int line, const char *file, const char *format, ...)
{
	va_list args;
	time_t t;
	struct tm tm;

	t = time(NULL);
	localtime_r(&t, &tm);

	if (log_file) {
		fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d %s;%03d: ",
			1900 + tm.tm_year, tm.tm_mon, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			file, line);

		va_start(args, format);
		vfprintf(log_file, format, args);
		va_end(args);

		fputc('\n', log_file);
	}
}
