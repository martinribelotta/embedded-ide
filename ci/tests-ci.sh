#!/bin/bash

set -x

source /opt/qt*/bin/qt*-env.sh
/opt/qt*/bin/qmake CONFIG+=release CONFIG+=force_debug_info embedded-ide.pro
make -j

set -e
