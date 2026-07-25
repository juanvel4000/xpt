#!/bin/sh
set -e
# xpt-make
#  create .xpt packages easily

createTreefile() {
    if [ ! -d "$1" ]; then
        echo "directory $1 does not exist" >&2
        return 1
    fi

    dir="$1"
    tmpd=""

    rm -f "$dir/xpt.tree"
    if [ -f "$dir/xpt.manifest" ]; then
        tmpd=$(mktemp)
        mv "$dir/xpt.manifest" "$tmpd"
    fi
    tree=$(cd "$dir" && find . \( -type f -o -type l \) | sed 's|^\./||')
    echo "$tree" > "$dir/xpt.tree"
    if [ -n "$tmpd" ] && [ -f "$tmpd" ]; then
        mv "$tmpd" "$dir/xpt.manifest"
    fi
    echo "successfully created a treefile for $dir"
}

createManifest() {
    if [ ! -d "$1" ]; then
        echo "directory $1 does not exist" >&2
        return 1
    fi
    dir="$1"
    rm -f "$dir/xpt.manifest"

    echo "xpt.manifest creator"
    echo "========================"
    printf "package name: "
    read name
    printf "package version: "
    read version
    printf "short description: "
    read desc
    printf "architecture: "
    read arch
    printf "maintainer: "
    read maintainer
    printf "dependencies (comma-separated list): "
    read dependencies

    echo "name=$name"              > "$dir/xpt.manifest"
    echo "version=$version"       >> "$dir/xpt.manifest"
    echo "desc=$desc"             >> "$dir/xpt.manifest"
    echo "arch=$arch"             >> "$dir/xpt.manifest"
    echo "maintainer=$maintainer" >> "$dir/xpt.manifest"
    echo "depends=$dependencies" >> "$dir/xpt.manifest"
    echo "successfully created a xpt.manifest for $dir"
}

createTarball() {
    if [ ! -d "$1" ]; then
        echo "directory $1 does not exist" >&2
        return 1
    fi
    dir="$1"

    if [ ! -f "$dir/xpt.manifest" ]; then
        echo "xpt.manifest does not exist" >&2
        return 1
    fi
    if [ ! -f "$dir/xpt.tree" ]; then
        echo "xpt.tree does not exist" >&2
        return 1
    fi
    PKG_NAME=$(grep '^name=' "$dir/xpt.manifest" | cut -d= -f2)
    PKG_VER=$(grep '^version=' "$dir/xpt.manifest" | cut -d= -f2)

    tar -czf "${PKG_NAME}-${PKG_VER}.xpt" -C "$dir" .
}

dir="$2"
dir="$(printf '%s' "$dir" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
case "$1" in
    -h)
        echo "xpt-make - package creator for xpt"
        echo "usage: $0 <cmd>"
        echo "commands"
        echo " -c <dir>     create a new xpt package based on <dir>"
        echo " -t <dir>     create a treefile for <dir>"
        echo " -m <dir>     create a manifest for <dir>"
        echo "xpt-make is licensed under the 3-clause BSD license"
        exit 0

        ;;
    -c)
        if [ -z "$dir" ]; then
            echo "usage: $0 -c <dir>"
            exit 1
        fi
        if [ ! -d "$dir" ]; then
            echo "directory $dir does not exist"
            exit 1
        fi
        createTreefile "$dir"
        createManifest "$dir"
        createTarball  "$dir"
        exit 0

        ;;

    -t)
        if [ -z "$dir" ]; then
            echo "usage: $0 -t <dir>"
            exit 1
        fi
        if [ ! -d "$dir" ]; then
            echo "directory $dir does not exist"
            exit 1
        fi
        createTreefile "$dir"
        ;;
    -m)
        if [ -z "$dir" ]; then
            echo "usage: $0 -m <dir>"
            exit 1
        fi
        if [ ! -d "$dir" ]; then
            echo "directory $dir does not exist"
            exit 1
        fi
        createManifest "$dir"
        ;;
    *)
        echo "usage: $0 <cmd>" >&2
        exit 1

        ;;
esac
