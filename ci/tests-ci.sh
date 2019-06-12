#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh
qmake --version

INSTALL_DIR=/tmp/embedded-ide
APP_IMAGE_NAME=Embedded_IDE-x86_64.AppImage
DEPLOY_OPT=-no-translations -verbose=2 -executable=$INSTALL_DIR/usr/bin/embedded-ide
DESKTOP_FILE=$INSTALL_DIR/usr/share/applications/embedded-ide.desktop

wget https://github.com/martinribelotta/embedded-ide-builder/blob/master/linux-x86_64/universal-ctags?raw=true -O /tmp/universal-ctags

echo ************** LINUX BUILD ***********************

/opt/qt*/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j
make install INSTALL_ROOT=${INSTALL_DIR}
linuxdeployqt $DESKTOP_FILE $DEPLOY_OPT -appimage
linuxdeployqt $INSTALL_DIR/usr/bin/qtshdialog -no-translations -verbose=2 -no-plugins
linuxdeployqt $INSTALL_DIR/usr/bin/socketwaiter -no-translations -verbose=2 -no-plugins
cp /opt/qt*/lib/libQt5Svg.so.5 $INSTALL_DIR/usr/lib
cp /opt/qt*/plugins/imageformats/libqsvg.so $INSTALL_DIR/usr/plugins/imageformats/
cp /tmp/universal-ctags $INSTALL_DIR/usr/bin
chmod a+x $INSTALL_DIR/usr/bin/universal-ctags
linuxdeployqt $DESKTOP_FILE -appimage

echo ************** WINDOWS BUILD ***********************

make distclean
i686-w64-mingw32.static-qmake-qt5  CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j
mv build embedded-ide
zip -9 -r ./Embedded_IDE-win32.zip embedded-ide

set -e
