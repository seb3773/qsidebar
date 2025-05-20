#!/bin/sh

rm -rf admin
rm -rf autom4te.cache
tar -xzvf /opt/trinity/share/apps/kdevappwizard/template-common/admin.tar.gz
tar -xzvf /opt/trinity/share/apps/kdevappwizard/template-common/gnu.tar.gz

read -p "Press \"Enter\" to continue .." xyz
WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" LC_MESSAGES="C" LC_CTYPE="C" [ ! -f /usr/share/libtool/ltmain.sh ] || cp -f /usr/share/libtool/ltmain.sh admin/ltmain.sh && [ ! -f /usr/share/libtool/config/ltmain.sh ] || cp -f /usr/share/libtool/config/ltmain.sh admin/ltmain.sh && cp -f /usr/share/aclocal/libtool.m4 admin/libtool.m4.in

read -p "Press \"Enter\" to continue .." xyz
if [ -f /usr/share/libtool/build-aux/ltmain.sh ] ; then
  cp /usr/share/libtool/build-aux/ltmain.sh admin/ltmain.sh
fi

read -p "Press \"Enter\" to continue .." xyz
WANT_AUTOCONF_2_5="1" WANT_AUTOMAKE_1_6="1" LC_MESSAGES="C" LC_CTYPE="C" make -f Makefile.cvs

read -p "Press \"Enter\" to configure .." xyz
./configure

read -p "Press \"Enter\" to make .." xyz
make

read -p "Press \"Enter\" to create package .." xyz
checkinstall --pkgname kickerapplet --pkgversion 0.1 --nodoc
