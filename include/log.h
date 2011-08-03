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

#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open the file for appending the log output.
 * @param file The filename of the log file to open.
 */
int log_open(const char *file);

/**
 * Open the file for appending the log output, but look for the filename in the
 * argument vector. -l writes the log to stderr, -l filename opens the filename
 * as log file.
 * @param argc The argument count (argc) from main.
 * @param argv The argument vector (argv) from main.
 */
int log_open_argv(int argc, char **argv);

/**
 * Close the log file if it is open.
 */
int log_close();

/**
 * Write debug entry to log. This is the internal helper function, call
 * log_debug instead.
 * @line Line in the source code file (provided by log_debug)
 * @file Source doe file name (provided by log_debug)
 */
void log_debug_(int line, const char *file, const char *format, ...);

/**
 * Write debug entry to log.
 */
#define log_debug(...) log_debug_(__LINE__, __FILE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOG_H */
