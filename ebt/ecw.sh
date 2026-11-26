#!/bin/sh

set -eu

# ecw chroot wrapper
# a script to chroot safely to a directory

mount_bind() {
    [ -d "$2" ] || mkdir -p "$2"
    mount --rbind "$1" "$2" || {
        echo "! cannot mount '$1' to '$2'" >&2
        exit 1
    }

    mount --make-rslave "$2" || {
        echo "! cannot make '$2' recursively slave" >&2
        exit 1
    }
}

unmount() {
    umount -R "$1" || {
        echo "! error unmounting '/$(basename "$1")'" >&2
        exit 1
    }
}

unmount_all() {
    unmount "$1/run"
    unmount "$1/tmp"
    unmount "$1/sys"
    unmount "$1/proc"
    unmount "$1/dev"
}

mount_all() {
    mount_bind "/dev" "$1/dev"
    mount_bind "/sys" "$1/sys"

    mount -t tmpfs tmpfs "$1/tmp"
    mount -t tmpfs tmpfs "$1/run"
    mount -t proc proc "$1/proc"

    [ -d "/sys/firmware/efi/efivars" ] && {
        mkdir -p "$1/sys/firmware/efi/efivars"
        mount -t efivarfs efivarfs "$1/sys/firmware/efi/efivars"
    }
}
do_chroot() {
    [ -d "$1" ] || mkdir -p "$1"

    ecw_shell="${2:-/bin/sh}"

    [ -f "$1/$ecw_shell" ] || {
        echo "! '$1/${ecw_shell#/}' does not exist" >&2
        exit 1
    }

    chroot_dir="$1"
    trap 'unmount_all "$chroot_dir"' EXIT
    trap 'exit 1' HUP INT TERM

    mount_all "$1"

    chroot "$1" "$ecw_shell"
}

[ -n "$1" ] || {
    echo "usage: ecw [command] <chroot>"
    exit 1
}

case "${1:-}" in
*)
    [ "$(id -u)" -ne 0 ] && {
        echo "! run as root" >&2
        exit 1
    }

    [ -n "${1:-}" ] || {
        echo "usage: ecw chroot [shell]" >&2
        exit 1
    }

    [ -d "$1" ] || {
        echo "'$1' does not exist" >&2
        exit 1
    }

    if [ -z "${2:-}" ]; then
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

        [ -n "${ecw_shell:-}" ] || {
            echo "! no shell found in '$2'" >&2
            exit 1
        }
    else
        if [ -f "$1/$2" ]; then
            ecw_shell="$2"
        else
            echo "! shell '$2' not found in '$1'" >&2
            exit 1
        fi
    fi
    do_chroot "$1" "$ecw_shell"
    ;;
esac
