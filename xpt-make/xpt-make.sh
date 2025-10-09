#!/bin/sh
set -e
set -o pipefail
# xpt-make
#  create .xpt packages easily

createTreefile() {
    if [ ! -d "$1" ]; then
        echo "directory $1 does not exist" >&2
        return 1 
    fi
    dir="$1"
    rm -f "$dir/xpt.tree"

    (cd "$dir" && find . \( -type f -o -type l \) | sed 's|^\./||') > "$dir/xpt.tree"
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
    read -p "package name: "        name
    read -p "package version: "     version
    read -p "short description: "   desc
    read -p "architecture: "        arch
    read -p "maintainer: "          maintainer

    echo "name=$name"              > "$dir/xpt.manifest"
    echo "version=$version"       >> "$dir/xpt.manifest"
    echo "desc=$desc"             >> "$dir/xpt.manifest"
    echo "arch=$arch"             >> "$dir/xpt.manifest"
    echo "maintainer=$maintainer" >> "$dir/xpt.manifest"
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

    tar -czf $(PKG_NAME)-$(PKG_VER).xpt -C "$dir" .
}
case "$1" in
    -h)
        echo "xpt-make - package creator for xpt"
        echo "usage: $0 <cmd>"
        echo "commands"
        echo " -c <dir>  create a new xpt package"
        echo "xpt-make is licensed under the 3-clause BSD license"
        exit 0
        
        ;;
    -c)
        if [ ! -d "$2" ]; then
            echo "directory $2 does not exist"
            exit 1
        fi
        createTreefile "$2"
        createManifest "$2"
        createTarball  "$2"
        exit 0
        
        ;;
    *)
        echo "usage: $0 <cmd>" >&2
        exit 1

        ;;
esac
