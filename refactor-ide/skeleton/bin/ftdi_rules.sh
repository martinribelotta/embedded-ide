#!/bin/bash

# Note that the following handles 0, 1 or more arguments (file paths)
# which can include blanks but uses a bashism; can the same be achieved
# in POSIX-shell? (FIXME)
# http://stackoverflow.com/questions/3190818
atexit()
{
  if [ -z "$SKIP" ] ; then
    if [ $NUMBER_OF_ARGS -eq 0 ] ; then
      exec "${BIN}"
    else
      exec "${BIN}" "${args[@]}"
    fi
  fi
}

error()
{
  if [ -x /usr/bin/zenity ] ; then
    LD_LIBRARY_PATH="" zenity --error --text "${1}" 2>/dev/null
  elif [ -x /usr/bin/kdialog ] ; then
    LD_LIBRARY_PATH="" kdialog --msgbox "${1}" 2>/dev/null
  elif [ -x /usr/bin/Xdialog ] ; then
    LD_LIBRARY_PATH="" Xdialog --msgbox "${1}" 2>/dev/null
  else
    echo "${1}"
  fi
  exit 1
}

msg()
{
  if [ -x /usr/bin/zenity ] ; then
    LD_LIBRARY_PATH="" zenity --info --text "${1}" 2>/dev/null
  elif [ -x /usr/bin/kdialog ] ; then
    LD_LIBRARY_PATH="" kdialog --msgbox "${1}" 2>/dev/null
  elif [ -x /usr/bin/Xdialog ] ; then
    LD_LIBRARY_PATH="" Xdialog --msgbox "${1}" 2>/dev/null
  else
    echo "${1}"
  fi
}

if [ "$(id -u)" -ne "0" ]; then
    echo "switching to root"
    # move outside path because root cannot access fuse mounted dirs
    NEWFILE=$(tempfile -s .sh)
    cp $(readlink -f $0) $(NEWFILE)
    chmod a+x $(NEWFILE)
    exec pkexec $(NEWFILE) $*
else
    if [ "$1" = "--install" ]; then
        echo "Install FTDI drivers"
        OOCD_RULE=$(<<EOF
ACTION!="add|change", GOTO="openocd_rules_end"
SUBSYSTEM!="usb", GOTO="openocd_rules_end"
ENV{DEVTYPE}!="usb_device", GOTO="openocd_rules_end"
ATTRS{idVendor}=="0406", ATTRS{idProduct}=="6010", MODE="666", GROUP="plugdev"
LABEL="openocd_rules_end"
EOF
        echo ${OOCD_RULE} > /etc/udev/rules.d/60-openocd.rules &&
        udevadm control -R &&
        msg "Install FTDI driver OK" ||
        msg "Install FTDI driver FAIL"
        exit 0
    elif [ "$1" = "--uninstall" ]; then
        echo "Uninstall FTDI drivers"
        rm -f /etc/udev/rules.d/60-openocd.rules &&
        udevadm control -R &&
        msg "Uninstall FTDI driver OK" ||
        msg "Uninstall FTDI driver FAIL";
        exit 0
    fi
fi
