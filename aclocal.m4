dnl @synopsis BNV_HAVE_QT [--with-Qt-includes=DIR] [--with-Qt-dir=DIR]
dnl
dnl Search for Trolltech's Qt GUI framework.
dnl
dnl Searches common directories for Qt include files, libraries and the
dnl meta object compiler. If one of the options is given, the search is
dnl started in the given directory. If both options are given, no checks
dnl are performed and the cache is not consulted nor altered.
dnl
dnl The following shell variable is set to either "yes" or "no":
dnl
dnl   have_qt
dnl
dnl Additionally, the following variables are exported:
dnl
dnl   QT_CXXFLAGS
dnl   QT_LIBS
dnl   QT_MOC
dnl   QT_DIR
dnl
dnl which contain an "-I" flag pointing to the Qt include directory, link flags
dnl necessary to link with Qt and X, the name of the meta object compiler
dnl with full path, and the variable QTDIR as Trolltech likes to see it
dnl defined, respectively.
dnl
dnl Example lines for Makefile.in:
dnl
dnl   CXXFLAGS = @QT_CXXFLAGS@
dnl   MOC      = @QT_MOC@
dnl
dnl After the variables have been set, a trial compile and link is
dnl performed to check the correct functioning of the meta object compiler.
dnl If this test fails, a warning appears in the output of configure, but
dnl the variables remain defined.
dnl
dnl No common variables such as $LIBS or $CFLAGS are polluted.
dnl
dnl Options:
dnl
dnl --with-Qt-includes=DIR: DIR should point to where the Qt
dnl includes can be found, as in -IDIR.
dnl
dnl --with-Qt-dir=DIR: DIR should point to the "QTDIR", i.e.,
dnl libraries are in DIR/lib/.
dnl
dnl If at least one of the options "=no" or, equivalently,
dnl --without-Qt-includes and/or --without-Qt-dir is given, "have_qt" is set
dnl to "no" and the other variables are set to the empty string.
dnl
dnl @version $Id: aclocal.m4,v 1.1 2001/06/08 16:56:07 cschmitt Exp $
dnl @author Bastiaan N. Veelo <Bastiaan.N.Veelo@immtek.ntnu.no>
dnl
dnl Calls BNV_PATH_QT_DIRECT as a subroutine.
dnl Copyright 2001 Bastiaan N. Veelo <Bastiaan.N.Veelo@immtek.ntnu.no>
AC_DEFUN([BNV_HAVE_QT],
[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])

  AC_MSG_CHECKING(for Qt)

  AC_ARG_WITH([Qt-includes],
    [  --with-Qt-includes=DIR  Qt include files are in DIR])
  AC_ARG_WITH([Qt-dir],
    [  --with-Qt-dir=DIR       Qt library file is in DIR/lib/])
  if test x"$with_Qt_includes" = x"no" || test x"$with_Qt_dir" = x"no"; then
    # user disabled Qt. Leave cache alone.
    have_qt="User disabled Qt."
  else
    # "yes" is a bogus option
    if test "x$with_Qt_includes" = xyes; then
      with_Qt_includes=
    fi
    if test "x$with_Qt_dir" = xyes; then
      with_Qt_dir=
    fi
    if test "x$with_Qt_includes" != x && test "x$with_Qt_dir" != x; then
      # Both values are set, no need to search
      have_qt=yes
      bnv_qt_includes="$with_Qt_includes"
      bnv_qt_dir="$with_Qt_dir"
      bnv_qt_LIBS="-lqt $X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt -lXi $X_EXTRA_LIBS"
      if test x"$with_Qt_dir" != x; then
        bnv_qt_LIBS="-L$with_Qt_dir/lib $bnv_qt_LIBS"
      fi
    else
      # Use cached value or do search, starting with suggestions from
      # the command line
      AC_CACHE_VAL(bnv_cv_have_qt,
      [
        # One or both of the vars are not set, and there is no cached value.
        bnv_qt_includes=NO bnv_qt_dir=NO
        BNV_PATH_QT_DIRECT
        if test "$bnv_qt_includes" = NO || test "$bnv_qt_dir" = NO; then
          # Didn't find Qt anywhere.  Cache the known absence of Qt.
          bnv_cv_have_qt="have_qt=no"
        else
          # Record where we found Qt for the cache.
          bnv_cv_have_qt="have_qt=yes              \
                  bnv_qt_includes=$bnv_qt_includes \
                       bnv_qt_dir=$bnv_qt_dir      \
                      bnv_qt_LIBS=\"$bnv_qt_LIBS\""
        fi
      ])dnl
      eval "$bnv_cv_have_qt"
    fi # all $bnv_qt_* are set
  fi   # $have_qt reflects the system status
  if test x"$have_qt" = xyes; then
    QT_CXXFLAGS="-I$bnv_qt_includes"
    QT_DIR="$bnv_qt_dir"
    QT_LIBS="$bnv_qt_LIBS"
    QT_MOC="$bnv_qt_dir/bin/moc"
    AC_MSG_RESULT([$have_qt:
    QT_CXXFLAGS=$QT_CXXFLAGS
    QT_LIBS=$QT_LIBS
    QT_MOC=$QT_MOC])
  else
    QT_CXXFLAGS=
    QT_DIR=
    QT_LIBS=
    QT_MOC=
    AC_MSG_RESULT($have_qt)
  fi
  AC_SUBST(QT_CXXFLAGS) AC_SUBST(QT_DIR) AC_SUBST(QT_LIBS) AC_SUBST(QT_MOC)

  #### Being paranoid:
  AC_MSG_CHECKING(the Qt meta object compiler)
  AC_CACHE_VAL(bnv_cv_qt_test_result,
  [
    cat > bnv_qt_test.h << EOF
#include <qobject.h>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

    cat > bnv_qt_main.$ac_ext << EOF
#include "bnv_qt_test.h"
#include <qapplication.h>
int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  Test t;
  QObject::connect( &t, SIGNAL(send()), &t, SLOT(receive()) );
}
EOF

    bnv_cv_qt_test_result="failure"
    bnv_try_1="$QT_MOC bnv_qt_test.h -o moc_bnv_qt_test.$ac_ext >/dev/null 2>bnv_qt_test_1.out"
    AC_TRY_EVAL(bnv_try_1)
    bnv_err_1=`grep -v '^ *+' bnv_qt_test_1.out | grep -v "^bnv_qt_test.h\$"`
    if test -n "$bnv_err_1"; then
      echo "$bnv_err_1" >&AC_FD_CC
      echo "configure: could not run $QT_MOC on:" >&AC_FD_CC
      cat bnv_qt_test.h >&AC_FD_CC
    else
      bnv_try_2="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o moc_bnv_qt_test.o moc_bnv_qt_test.$ac_ext >/dev/null 2>bnv_qt_test_2.out"
      AC_TRY_EVAL(bnv_try_2)
      bnv_err_2=`grep -v '^ *+' bnv_qt_test_2.out | grep -v "^bnv_qt_test.{$ac_ext}\$"`
      if test -n "$bnv_err_2"; then
        echo "$bnv_err_2" >&AC_FD_CC
        echo "configure: could not compile:" >&AC_FD_CC
        cat bnv_qt_test.$ac_ext >&AC_FD_CC
      else
        bnv_try_3="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o bnv_qt_main.o bnv_qt_main.$ac_ext >/dev/null 2>bnv_qt_test_3.out"
        AC_TRY_EVAL(bnv_try_3)
        bnv_err_3=`grep -v '^ *+' bnv_qt_test_3.out | grep -v "^bnv_qt_main.{$ac_ext}\$"`
        if test -n "$bnv_err_3"; then
          echo "$bnv_err_3" >&AC_FD_CC
          echo "configure: could not compile:" >&AC_FD_CC
          cat bnv_qt_main.$ac_ext >&AC_FD_CC
        else
          bnv_try_4="$CXX $QT_LIBS $LIBS -o bnv_qt_main bnv_qt_main.o moc_bnv_qt_test.o >/dev/null 2>bnv_qt_test_4.out"
          AC_TRY_EVAL(bnv_try_4)
          bnv_err_4=`grep -v '^ *+' bnv_qt_test_4.out`
          if test -n "$bnv_err_4"; then
            echo "$bnv_err_4" >&AC_FD_CC
          else
            bnv_cv_qt_test_result="succes"
          fi
        fi
      fi
    fi
  ])dnl AC_CACHE_VAL bnv_cv_qt_test_result
  AC_MSG_RESULT([$bnv_cv_qt_test_result]);
  if test x"$bnv_cv_qt_test_result" = "xfailure"; then
    AC_MSG_WARN([Your Qt installation seems to be broken!])
  fi

  rm -f bnv_qt_test.h bnv_qt_test.$ac_ext bnv_qt_test.o \
        bnv_qt_main.$ac_ext bnv_qt_main.o bnv_qt_main \
        bnv_qt_test_1.out bnv_qt_test_2.out bnv_qt_test_3.out bnv_qt_test_4.out
])

dnl Internal subroutine of BNV_HAVE_QT
dnl Set bnv_qt_includes bnv_qt_dir.
dnl Copyright 2001 Bastiaan N. Veelo <Bastiaan.N.Veelo@immtek.ntnu.no>
AC_DEFUN(BNV_PATH_QT_DIRECT,
[
  ## Look for include files ##
  qt_direct_test_header=qapplication.h
  # First, try using that file with no special directory specified.
  AC_TRY_CPP([#include <$qt_direct_test_header>],
  [
    # Success.
    # We can compile using Qt headers with no special include directory.
    bnv_qt_includes=
  ], [
    # That did not work.
    echo "Non-critical error, please neglect the above." >&AC_FD_CC
    # Look for the header file in a standard set of common directories.
    bnv_include_path_list="
      $with_Qt_include
      /usr/include/qt
      /usr/lib/qt2/include
      /usr/lib/qt1g/include
      `ls -d /usr/local/qt*/include 2>/dev/null`
    "
    for bnv_dir in $bnv_include_path_list; do
      if test -r "$bnv_dir/$qt_direct_test_header"; then
        bnv_qt_includes=$bnv_dir
        break
      fi
    done
  ])

  ## Look for Qt directory ##
  qt_direct_test_library=qt
  qt_direct_test_main="
    int argc;
    char ** argv;
    QApplication app(argc,argv);
  "

  # See if we find the library without any special options.
  # Don't add top $LIBS permanently yet
  bnv_save_LIBS="$LIBS"
  LIBS="-l$qt_direct_test_library $X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt -lXi $X_EXTRA_LIBS"
  bnv_qt_LIBS="$LIBS"
  AC_TRY_LINK([#include <$qt_direct_test_header>],
    $qt_direct_test_main,
  [
    # Succes.
    # We can link with no special library directory.
    bnv_qt_dir=
  ], [
    # That did not work.
    echo "Non-critical error, please neglect the above." >&AC_FD_CC
    # Look for the Qt dir in a standard set of common directories.
    bnv_dir_list="
      $with_Qt_dir
      /usr
      /usr/lib/qt
      /usr/lib/qt2
      /usr/lib/qt1g
      `ls -d /usr/local/qt* 2>/dev/null`
    "
    for bnv_dir in $bnv_dir_list; do
      for bnv_ext in a so sl; do
        if test -r $bnv_dir/lib/lib${qt_direct_test_library}.$bnv_ext; then
          bnv_qt_dir=$bnv_dir
          break 2
        fi
      done
    done
  ])
  QT_DIR=$bnv_qt_dir
  if test -z $bnv_qt_dir; then
    QT_LIBS=$LIBS
    # when the library is in $LD_PATH then moc can be assumed to be in $PATH:
    QT_MOC=moc
  else
    QT_LIBS="-L$bnv_qt_dir/lib $LIBS"
    QT_MOC="$bnv_qt_dir/moc"
  fi
  LIBS="$bnv_save_LIBS"
])
