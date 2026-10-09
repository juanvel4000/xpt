#!/bin/sh
# ecw chroot wrapper
# a script to chroot safely to a directory

mount_bind() {
    [ -d "$2" ] || mkdir -p "$2"
    mount --rbind "$1" "$2" || {
        echo "cannnot mount $1 to $2"
        exit 1
    }
    mount --make-rslave "$2"
}

safe_umount() {
    umount -R "$1/dev" || {
        echo "! error umounting /dev"
        exit 1
    }
    umount -R "$1/proc" || {
        echo "! error umounting /proc"
        exit 1
    }
    umount -R "$1/sys" || {
        echo "! error umounting /sys"
        exit 1
    }
    umount -R "$1/tmp" || {
        echo "! error umounting /tmp"
        exit 1
    }
    umount -R "$1/run" || {
        echo "! error umounting /run"
        exit 1
    }
}

do_chroot() {
    [ -d "$1" ] || mkdir -p "$1"
    ecw_shell="${2:-/bin/sh}"
    [ -f "$1/$ecw_shell" ] || {
        echo "! $1/$ecw_shell does not exist"
        exit 1
    }
    mount_bind "/dev" "$1/dev"
    mount_bind "/sys" "$1/sys"
    mount_bind "/tmp" "$1/tmp"
    mount_bind "/run" "$1/run"
    mount -t proc proc "$1/proc"
    trap 'safe_umount $1' EXIT INT
    chroot "$1" "$ecw_shell" || {
        echo "! chroot exited"
        safe_umount "$1"
        exit 1
    }
}
[ -n "$1" ] || {
    echo "usage: ecw [command] <chroot>"
    exit 1
}
case "$1" in
help | -h | --help | h)
    echo "ecw"
    echo "usage: ecw [command] <chroot> [shell]"
    echo ""
    echo "safely chroot to a directory."
    echo ""
    echo "if no command is provided but a chroot is provided, it will open the chroot."
    echo "part of ebt build tools (ebt); see ecw(1) for more information"
    ;;
*)
    [ -d "$1" ] || {
        echo "$1 does not exist"
        exit 1
    }
    if ! [ -n "$2" ]; then
        for shell in \
            /bin/sh /usr/bin/sh /bin/bash /usr/bin/bash \
            /bin/dash /usr/bin/dash /bin/zsh /usr/bin/zsh \
            /bin/fish /usr/bin/fish /bin/ksh /usr/bin/ksh \
            /bin/csh /usr/bin/csh /bin/tcsh /usr/bin/tcsh \
            /bin/ash /usr/bin/ash /bin/busybox /usr/bin/busybox \
            /bin/mksh /usr/bin/mksh /bin/yash /usr/bin/yash \
            /bin/elvish /usr/bin/elvish /bin/xonsh /usr/bin/xonsh \
            /bin/rc /usr/bin/rc /bin/shush /usr/bin/shush; do
            if [ -f "$1/$shell" ]; then
                ecw_shell="$shell"
                break
            fi
        done
        [ -n "$ecw_shell" ] || {
            echo "! no shell found"
            exit 1
        }
    else
        if [ -f "$1/$2" ]; then
            ecw_shell="$2"
        else
            echo "! shell $2 not found in $1"
            exit 1
        fi
    fi
    do_chroot "$1" "$ecw_shell"
    ;;
esac
