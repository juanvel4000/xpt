#!/bin/sh
# mkfhs: create a fhs-compiliant tree layout
set -e


ROOTFS=${1:-$(pwd)}
TARGETARCH=${2:-$(uname -m)}
if [ "$ROOTFS" = '--help' ]; then
    echo "usage: $0 [rootfs] [arch]"
    echo "create a fhs-compiliant directory tree in [rootfs]"
    echo "  [rootfs]      defaults to '.'"
    echo "  [arch]        defaults to $(uname -m)"
    echo "  if [arch] is 'x86_64', then a lib64 directory will be created"
    echo "part of ebt build tools (ebt)"
    exit 0
fi

echo "* creating a $TARGETARCH structure on $ROOTFS"
mkdir -p "$ROOTFS"

# create core structure
mkdir -p "$ROOTFS/usr/bin" "$ROOTFS/usr/sbin" "$ROOTFS/usr/share/man" "$ROOTFS/usr/share/info" \
         "$ROOTFS/usr/include" "$ROOTFS/usr/src" "$ROOTFS/usr/local/bin" "$ROOTFS/usr/local/sbin" \
         "$ROOTFS/usr/local/include" "$ROOTFS/usr/local/share" "$ROOTFS/usr/local/lib" \
         "$ROOTFS/var/cache" "$ROOTFS/var/lib" "$ROOTFS/var/log" "$ROOTFS/var/tmp" "$ROOTFS/var/spool" \
         "$ROOTFS/etc" "$ROOTFS/run" "$ROOTFS/tmp" "$ROOTFS/dev" "$ROOTFS/proc" "$ROOTFS/sys" \
         "$ROOTFS/mnt" "$ROOTFS/media" "$ROOTFS/home" "$ROOTFS/root" "$ROOTFS/srv/www" \
         "$ROOTFS/srv/ftp" "$ROOTFS/opt" "$ROOTFS/boot" "$ROOTFS/usr/lib" "$ROOTFS/var/opt" \
         "$ROOTFS/var/lock" "$ROOTFS/var/mail" "$ROOTFS/var/local" "$ROOTFS/var/local/lib" \
         "$ROOTFS/var/local/cache" "$ROOTFS/usr/local/games" "$ROOTFS/usr/libexec" \
         "$ROOTFS/usr/games" "$ROOTFS/usr/local/etc" "$ROOTFS/etc/opt" "$ROOTFS/etc/X11" \
         "$ROOTFS/etc/xml" "$ROOTFS/etc/skel" "$ROOTFS/usr/local/libexec"

case "$TARGETARCH" in
    x86_64)
        echo "* creating lib64"
        mkdir -p "$ROOTFS/usr/lib64"
        ln -sr "$ROOTFS/usr/lib64" "$ROOTFS/lib64"
        ;;
esac
echo "* symlinking sbin, bin and lib to root"
(
    cd "$ROOTFS"
    ln -sr "$ROOTFS/usr/lib" "$ROOTFS/lib"
    ln -sr "$ROOTFS/usr/bin" "$ROOTFS/bin"
    ln -sr "$ROOTFS/usr/sbin" "$ROOTFS/sbin"
)

echo "* setting permissions"
chmod 1777 "$ROOTFS/tmp" "$ROOTFS/var/tmp"
chmod 0700 "$ROOTFS/root"

echo "successfully created a fhs tree in $ROOTFS for $TARGETARCH"
