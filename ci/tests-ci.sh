#!/bin/bash

set -x
BASE=$(dirname $(readlink -f $0))

source /opt/qt*/bin/qt*-env.sh

set -e

qmake --version

VERSION=$(git rev-parse --short HEAD)

INSTALL_DIR=/tmp/embedded-ide
APP_IMAGE_NAME=Embedded_IDE-x86_64.AppImage
DEPLOY_OPT="-no-translations -verbose=2 -executable=$INSTALL_DIR/usr/bin/embedded-ide"
DESKTOP_FILE=$INSTALL_DIR/usr/share/applications/embedded-ide.desktop

wget https://github.com/martinribelotta/embedded-ide-builder/blob/master/linux-x86_64/universal-ctags?raw=true -O /tmp/universal-ctags

echo ************** LINUX BUILD ***********************

/opt/qt*/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j4
make install INSTALL_ROOT=${INSTALL_DIR}
install ${BASE}/embedded_ide-config.json.unix ${INSTALL_DIR}/usr/share/embedded-ide/embedded_ide-config.json
install ${BASE}/embedded-ide.hardconf.unix ${INSTALL_DIR}/usr/share/embedded-ide/embedded-ide.hardconf
linuxdeployqt $DESKTOP_FILE $DEPLOY_OPT -appimage
cp /opt/qt*/lib/libQt5Svg.so.5 $INSTALL_DIR/usr/lib
cp /opt/qt*/lib/libQt5Qml.so.5 $INSTALL_DIR/usr/lib
cp /opt/qt*/plugins/imageformats/libqsvg.so $INSTALL_DIR/usr/plugins/imageformats/
instal -m 0755 /tmp/universal-ctags $INSTALL_DIR/usr/bin
linuxdeployqt $DESKTOP_FILE -appimage
(
APPIMAGE=${PWD}/Embedded_IDE-${VERSION}-x86_64.AppImage
cd /tmp
chmod a+x ${APPIMAGE}
${APPIMAGE} --appimage-extract
mv squashfs-root Embedded_IDE-${VERSION}-x86_64
tar -jcvf ${BASE}/Embedded_IDE-${VERSION}-x86_64.tar.bz2 Embedded_IDE-${VERSION}-x86_64
)

echo ************** WINDOWS BUILD ***********************

make distclean
export MXE_PREFIX=i686-w64-mingw32.shared
export MXE=/usr/lib/mxe/usr
export MXEQT=${MXE}/i686-w64-mingw32.shared/qt5
export PATH=${MXE}/bin:${PATH}
${MXEQT}/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j4
pydeployqt --objdump ${MXE_PREFIX}-objdump ${PWD}/build/embedded-ide.exe \
	--libs ${MXE}/${MXE_PREFIX}/bin/:${MXEQT}/bin/:${MXEQT}/lib/ \
	--extradll Qt5Svg.dll:Qt5Qml.dll:libjpeg-9.dll \
	--qmake ${MXEQT}/bin/qmake
install ${BASE}/embedded_ide-config.json.win build/embedded_ide-config.json
install ${BASE}/embedded-ide.hardconf.win build/embedded-ide.hardconf
mv build embedded-ide
zip -9 -r Embedded_IDE-${VERSION}-win32.zip embedded-ide
