#!/bin/sh
set -e

DEBUG=
# Be verbose if $DEBUG=1 is set
if [ ! -z "$DEBUG" ] ; then
  env
  set -x
fi

THIS="$0"
args=$(echo "$@") # http://stackoverflow.com/questions/3190818/
NUMBER_OF_ARGS="$#"

# Please do not change $VENDORPREFIX as it will allow for desktop files
# belonging to AppImages to be recognized by future AppImageKit components
# such as desktop integration daemons
VENDORPREFIX=appimagekit

if [ -z $APPDIR ] ; then
    # Find the AppDir. It is the directory that contains AppRun.
    # This assumes that this script resides inside the AppDir or a subdirectory.
    # If this script is run inside an AppImage, then the AppImage runtime
    # likely has already set $APPDIR
    APPDIR=$(readlink -f $(dirname $(readlink -f $THIS))/../../)
fi

FILENAME="$(readlink -f "${THIS}")"
DIRNAME=$(dirname $FILENAME)

DESKTOPFILE=$(find "$APPDIR" -maxdepth 1 -name "*.desktop" | head -n 1)
DESKTOPFILE_NAME=$(basename "${DESKTOPFILE}")

APP_FULL=$(sed -n -e 's/^Name=//p' "${DESKTOPFILE}" | head -n 1)
APP=$(echo "$APP_FULL" | tr -c -d '[:alnum:]')
if [ -z "$APP" ] || [ -z "$APP_FULL" ] ; then
    APP=$(echo "$DESKTOPFILE_NAME" | sed -e 's/.desktop//g')
    APP_FULL="$APP"
fi

check_dep() {
    DEP=$1
    if [ -z $(which $DEP) ] ; then
        echo "$DEP is missing. Skipping ${THIS}."
        exit 0
    fi
}

# Determine where the desktop file should be installed
if [ $(id -u) -ne 0 ]; then
    DESTINATION_DIR_DESKTOP="$HOME/.local/share/applications"
    STAMP_DIR="$HOME/.local/share/$VENDORPREFIX"
    SYSTEM_WIDE=""
else
    # TODO: Check $XDG_DATA_DIRS
    DESTINATION_DIR_DESKTOP="/usr/local/share/applications"
    STAMP_DIR="/etc/$VENDORPREFIX"
    SYSTEM_WIDE="--mode system" # for xdg-mime and xdg-icon-resource
fi

msgbox() {
    qtdialog --msgbox "$1"
}

yesno() {
    qtdialog --yesno "$1" && RETURN="no" || RETURN="yes"
}

usage() {
    echo "Usage: $0 [--install | --uninstall]"
    exit 1
}

check_prevent()
{
  FILE=$1
  if [ -e "$FILE" ] ; then
    exit 0
  fi
}

uninstall_to_system() {
    rm -f "$STAMP_DIR/${APP}_no_desktopintegration" "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME"
    check_dep xdg-desktop-menu
    xdg-desktop-menu forceupdate
    exit 0
}

if [ $# -eq 0 ]; then
    usage
fi
if [ "x$1" = "x--install" ]; then
    echo "Log 1"
    # Exit immediately if one of these files is present
    # (e.g., because the desktop environment wants to handle desktop integration itself)
    check_prevent "$HOME/.local/share/$VENDORPREFIX/no_desktopintegration"
    echo "Log 2"
    check_prevent "/usr/share/$VENDORPREFIX/no_desktopintegration"
    echo "Log 3"
    check_prevent "/etc/$VENDORPREFIX/no_desktopintegration"
    echo "Log 4"
    if [ -z "$APPIMAGE" ] ; then
        APPIMAGE="$APPDIR/AppRun"
        # Not running from within an AppImage; hence using the AppRun for Exec=
    fi
    echo "Log 5"

    ICONFILE="$APPDIR/.DirIcon"

    # $XDG_DATA_DIRS contains the default paths /usr/local/share:/usr/share
    # desktop file has to be installed in an applications subdirectory
    # of one of the $XDG_DATA_DIRS components
    if [ -z "$XDG_DATA_DIRS" ] ; then
        echo "\$XDG_DATA_DIRS is missing. Please run ${THIS} from within an AppImage."
        exit 0
    fi
    echo "Log 6"

    # Check if the desktop file is already there
    # and if so, whether it points to the same AppImage
    if [ -e "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME" ] ; then
        # echo "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME already there"
        EXEC=$(grep "^Exec=" "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME" | head -n 1 | cut -d " " -f 1)
        # echo $EXEC
        if [ "Exec=\"$APPIMAGE\"" == "$EXEC" ] ; then
            exit 0
        fi
    fi
    echo "Log 7"

    # We ask the user only if we have found no reason to skip until here
    if [ -z "$SKIP" ] ; then
        yesno "Install" "Would you like to integrate $APPIMAGE with your system?\n\nThis will add it to your applications menu and install icons.\nIf you don't do this you can still launch the application by double-clicking on the AppImage."
    fi
    echo "Log 8"
    echo "Return: $RETURN"
    if [ "$RETURN" = "no" ] ; then
        yesno "Disable question?" "Should this question be permanently disabled for $APP?\n\nTo re-enable this question you have to delete\n\"$STAMP_DIR/${APP}_no_desktopintegration\""
        if [ "$RETURN" = "yes" ] ; then
            mkdir -p "$STAMP_DIR"
            touch "$STAMP_DIR/${APP}_no_desktopintegration"
        fi
        exit 0
    fi

    # desktop-file-install is supposed to install .desktop files to the user's
    # applications directory when run as a non-root user,
    # and to /usr/share/applications if run as root
    # but that does not really work for me...
    #
    # For Exec we must use quotes
    # For TryExec quotes is not supported, so, space must be replaced to \s
    # https://askubuntu.com/questions/175404/how-to-add-space-to-exec-path-in-a-thumbnailer-descrption/175567
    echo "Installing into system ${APPIMAGE}"
    RESOURCE_NAME=$(echo "$VENDORPREFIX-$DESKTOPFILE_NAME" | sed -e 's/.desktop//g')
    desktop-file-install --rebuild-mime-info-cache \
        --vendor=$VENDORPREFIX --set-key=Exec --set-value="\"${APPIMAGE}\" %U" \
        --set-key=X-AppImage-Comment --set-value="Generated by ${THIS}" \
        --set-icon="$RESOURCE_NAME" --set-key=TryExec --set-value=${APPIMAGE// /\\s} "$DESKTOPFILE" \
        --dir "$DESTINATION_DIR_DESKTOP"
    chmod a+x "$DESTINATION_DIR_DESKTOP/"*
    echo $RESOURCE_NAME
    echo $APP

    # delete "Actions" entry and add an "Uninstall" action
    echo $(date)
    sed -i -e "s/XXX_APP_FULL_XXX/$APP_FULL/" "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME"
    sed -i -e "s!XXX_APPIMAGE_XXX!\"$APPIMAGE\"!" "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME"
    #sed -i -e '/^Actions=/d' "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME"
    #cat >> "$DESTINATION_DIR_DESKTOP/$VENDORPREFIX-$DESKTOPFILE_NAME" << EOF
    #
    #
    #EOF

    # Install the icon files for the application; TODO: scalable
    ICONS=$(find "${APPDIR}" -iwholename "*/${APP}.png" 2>/dev/null || true)
    echo "Icons for ${APP} on ${APPDIR} ${ICONS}"
    for ICON in $ICONS ; do
        ICON_SIZE=256
        echo "Installing ${ICON} to size ${ICON_SIZE} on resource ${RESOURCE_NAME}"
        xdg-icon-resource install --context apps --size ${ICON_SIZE} "${ICON}" "${RESOURCE_NAME}"
    done

    # Install mime type
    find "${APPDIR}/usr/share/mime/" -type f -name *xml -exec xdg-mime install $SYSTEM_WIDE --novendor {} \; 2>/dev/null || true

    # Install the icon files for the mime type; TODO: scalable
    ICONS=$(find "${APPDIR}" -iwholename "*/mimetypes/*.png" 2>/dev/null || true)
    for ICON in $ICONS ; do
        ICON_SIZE=$(echo "${ICON}" | rev | cut -d "/" -f 3 | rev | cut -d "x" -f 1)
        xdg-icon-resource install --context mimetypes --size ${ICON_SIZE} "${ICON}" $(basename $ICON | sed -e 's/.png//g')
    done

    xdg-desktop-menu forceupdate
    gtk-update-icon-cache # for MIME
    msgbox "$APPIMAGE"
elif [ "x$1" = "x--uninstall" ]; then
    echo 
else
    echo "Unrecognized option $1"
    usage
fi
