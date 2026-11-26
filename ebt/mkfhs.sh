#!/bin/sh

set -eu

# mkfhs: create a filesystem layout compliant with
#
# usr-merge: https://wiki.debian.org/UsrMerge
# FHS: https://specifications.freedesktop.org/fhs/latest/

ROOTFS=${1:-}

[ "$#" -gt 1 ] && {
    echo "usage: mkfhs rootfs" >&2
    exit 1
}

echo "> creating a filesystem layout on '$ROOTFS'"
mkdir -p "$ROOTFS"

echo "> creating base directories"
mkdir -p "$ROOTFS/boot" "$ROOTFS/dev" "$ROOTFS/etc" "$ROOTFS/etc/opt" \
    "$ROOTFS/etc/X11" "$ROOTFS/etc/xml" "$ROOTFS/etc/skel" \
    "$ROOTFS/home" "$ROOTFS/media" "$ROOTFS/mnt" "$ROOTFS/opt" \
    "$ROOTFS/proc" "$ROOTFS/root" "$ROOTFS/run" "$ROOTFS/srv/www" \
    "$ROOTFS/srv/ftp" "$ROOTFS/sys" "$ROOTFS/tmp" \
    "$ROOTFS/usr/bin" "$ROOTFS/usr/sbin" "$ROOTFS/usr/lib" \
    "$ROOTFS/usr/libexec" "$ROOTFS/usr/lib/pkgconfig" \
    "$ROOTFS/usr/lib/modules" "$ROOTFS/usr/lib/firmware" \
    "$ROOTFS/usr/include" "$ROOTFS/usr/src" "$ROOTFS/usr/share" \
    "$ROOTFS/usr/share/man" "$ROOTFS/usr/share/man/man1" \
    "$ROOTFS/usr/share/man/man2" "$ROOTFS/usr/share/man/man3" \
    "$ROOTFS/usr/share/man/man4" "$ROOTFS/usr/share/man/man5" \
    "$ROOTFS/usr/share/man/man6" "$ROOTFS/usr/share/man/man7" \
    "$ROOTFS/usr/share/man/man8" "$ROOTFS/usr/share/man/man9" \
    "$ROOTFS/usr/share/info" "$ROOTFS/usr/share/doc" \
    "$ROOTFS/usr/share/licenses" "$ROOTFS/usr/share/misc" \
    "$ROOTFS/usr/share/locale" "$ROOTFS/usr/share/zoneinfo" \
    "$ROOTFS/usr/share/src" "$ROOTFS/usr/games" \
    "$ROOTFS/usr/local/bin" "$ROOTFS/usr/local/sbin" \
    "$ROOTFS/usr/local/include" "$ROOTFS/usr/local/share" \
    "$ROOTFS/usr/local/etc" "$ROOTFS/usr/local/games" \
    "$ROOTFS/var/cache" "$ROOTFS/var/lib" "$ROOTFS/var/lock" \
    "$ROOTFS/var/log" "$ROOTFS/var/mail" "$ROOTFS/var/opt" \
    "$ROOTFS/var/spool" "$ROOTFS/var/tmp" "$ROOTFS/var/local" \
    "$ROOTFS/var/local/cache" "$ROOTFS/var/local/lib" \
    "$ROOTFS/usr/local/src" "$ROOTFS/var/lib/misc" \
    "$ROOTFS/usr/local/man" "$ROOTFS/usr/local/lib"

[ -e "$ROOTFS/var/run" ] || [ -L "$ROOTFS/var/run" ] || ln -s ../run "$ROOTFS/var/run"

echo "> linking sbin, bin and lib to usr"
(
    cd "$ROOTFS"
    [ -e bin ] || [ -L bin ] || ln -s usr/bin bin
    [ -e sbin ] || [ -L sbin ] || ln -s usr/sbin sbin
    [ -e lib ] || [ -L lib ] || ln -s usr/lib lib
)

echo "> setting permissions"
chmod 1777 "$ROOTFS/tmp" "$ROOTFS/var/tmp"
chmod 0700 "$ROOTFS/root"

echo "> successfully created a filesystem tree in $ROOTFS"
