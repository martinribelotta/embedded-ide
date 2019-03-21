#!/bin/sh
APP_DIR=`dirname $0`
APP_DIR=`cd "${APP_DIR}";pwd`
export PATH=${APP_DIR}/bin:${PATH}
export LD_LIBRARY_PATH="${APP_DIR}/lib":
for v in $(env | grep QT_ | cut -f 1 -d '=')
do
        export $v=
done
exec "${APP_DIR}/bin/embedded-ide" "$@"
