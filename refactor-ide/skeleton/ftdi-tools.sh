#!/bin/bash
U=$(id -u)
echo "Execute as user $U"

# check for root
if [ $U -ne 0 ]; then
    echo "Re-exec as root..."
    tmpFile=$(tempfile -m 0755 -p ftdi-tools_ -s .sh)
    SELF=$(readlink -f $0)
    cat $SELF > $tmpFile
    pkexec $tmpFile $*
    rm $tmpFile
    exit 0
fi

if [ "x$1" = "x--install" ] ; then
  echo "Install FTDI drivers"
  UDEV_FILE=/etc/udev/rules.d/60-openocd.rules
  cat > $UDEV_FILE <<EOF
ACTION!="add|change", GOTO="openocd_rules_end"
SUBSYSTEM!="usb", GOTO="openocd_rules_end"
ENV{DEVTYPE}!="usb_device", GOTO="openocd_rules_end"
ATTRS{idVendor}=="0406", ATTRS{idProduct}=="6010", MODE="666", GROUP="plugdev"
LABEL="openocd_rules_end"
EOF
  udevadm control -R &&
  echo "Install FTDI driver OK" ||
  echo "Install FTDI driver FAIL"
  exit 0
elif [ "x$1" = "x--uninstall" ] ; then
  SKIP="yes"
  echo "Uninstall FTDI drivers"
  rm -f /etc/udev/rules.d/60-openocd.rules &&
  udevadm control -R &&
  echo "Uninstall FTDI driver OK" ||
  echo "Uninstall FTDI driver FAIL" ||
  exit 0
fi
