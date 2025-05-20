#!/bin/sh
make clean
make
if [ $? -eq 0 ]; then
strip ./.libs/qsidebar_applet_panelapplet.so
read -p "Press \"Enter to install libs & restart kicker\" ..." xyz
killall -w kicker
sudo \cp qsidebar_applet.desktop /opt/trinity/share/apps/kicker/applets/qsidebar_applet.desktop
sudo \cp .libs/qsidebar_applet_panelapplet.lai /opt/trinity/lib/trinity/qsidebar_applet_panelapplet.la
sudo \cp .libs/qsidebar_applet_panelapplet.so /opt/trinity/lib/trinity/qsidebar_applet_panelapplet.so
kicker &
echo "done."
else
echo "make error."
fi
