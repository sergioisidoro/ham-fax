dnl HamFax -- an application for sending and receiving amateur radio facsimiles
dnl Copyright (C) 2001--2003
dnl Christof Schmitt, DH1CS <cschmitt@users.sourceforge.net>
dnl  
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU General Public License
dnl as published by the Free Software Foundation; either version 2
dnl of the License, or (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
dnl
dnl Process this file with autoconf to produce a configure script.

AC_INIT(HamFax, 0.6.3, cschmitt@users.sourceforge.net)
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL
BNV_HAVE_QT

AH_TEMPLATE(USE_FILE, [Build with support for audio files.])
AC_CHECK_LIB(audiofile, afOpenFile,
                        [AC_DEFINE(USE_FILE) USE_FILE="1" LIBS="$LIBS -laudiofile"])
AM_CONDITIONAL(USE_FILE, test x$USE_FILE = x1)

AH_TEMPLATE(USE_OSS, [Build with support for the open sound system (OSS).])
AC_CHECK_HEADER(sys/soundcard.h, [AC_DEFINE(USE_OSS) USE_OSS="1"])
AM_CONDITIONAL(USE_OSS, test x$USE_OSS = x1)

AH_TEMPLATE(USE_PTC, [Build with support for the PTC-II])
AC_SYS_POSIX_TERMIOS
AM_CONDITIONAL(USE_PTC, test x$am_cv_sys_posix_termios = xyes)
if test x$am_cv_sys_posix_termios = xyes; then
   AC_DEFINE(USE_PTC)
fi

AC_PATH_PROG(LRELEASE, lrelease)
AM_CONDITIONAL(USE_LRELEASE, test -n $LRELEASE)

AC_OUTPUT(Makefile)
