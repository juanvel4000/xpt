#!/bin/sh
# mkfhs: create a hier(7) tree layout
set -e

ROOTFS=${1:-$(pwd)}
TARGETARCH=${2:-$(uname -m)}
if [ "$ROOTFS" = '--help' ]; then
    echo "mkfhs"
    echo "usage: mkfhs [rootfs] [arch]"
    echo ""
    echo "create a hier(7)-compiliant directory tree in [rootfs]"
    echo ""
    echo "  rootfs      defaults to '.'"
    echo "  arch        defaults to $(uname -m)"
    echo ""
    echo "if [arch] is 'x86_64', then a lib64 directory will be created."
    echo ""
    echo "part of ebt build tools (ebt); see mkfhs(1) for more information."
    exit 0
fi

echo "* creating a $TARGETARCH structure on $ROOTFS"
mkdir -p "$ROOTFS"

# create core structure
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
    "$ROOTFS/usr/local/lib" "$ROOTFS/usr/local/libexec" \
    "$ROOTFS/usr/local/include" "$ROOTFS/usr/local/share" \
    "$ROOTFS/usr/local/etc" "$ROOTFS/usr/local/games" \
    "$ROOTFS/var/cache" "$ROOTFS/var/lib" "$ROOTFS/var/lock" \
    "$ROOTFS/var/log" "$ROOTFS/var/mail" "$ROOTFS/var/opt" \
    "$ROOTFS/var/spool" "$ROOTFS/var/tmp" "$ROOTFS/var/local" \
    "$ROOTFS/var/local/cache" "$ROOTFS/var/local/lib"

case "$TARGETARCH" in
x86_64)
    echo "* creating lib64"
    mkdir -p "$ROOTFS/usr/lib64"
    ln -s usr/lib64 "$ROOTFS/lib64"
    ;;
esac
echo "* symlinking sbin, bin and lib to root"
(
    cd "$ROOTFS"
    [ -e bin ] || [ -L bin ] || ln -s usr/bin bin
    [ -e sbin ] || [ -L sbin ] || ln -s usr/sbin sbin
    [ -e lib ] || [ -L lib ] || ln -s usr/lib lib
)

echo "* setting permissions"
chmod 1777 "$ROOTFS/tmp" "$ROOTFS/var/tmp"
chmod 0700 "$ROOTFS/root"

echo "successfully created a hier(7) tree in $ROOTFS for $TARGETARCH"
