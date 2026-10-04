#!/bin/sh

set -eu

if [ "${1:-}" = "" ]; then
    echo "usage: fmt.sh [-mcsh]" >&2
    exit 1
fi
fmt_meson=false
fmt_c=false
fmt_h=false
fmt_sh=false

while getopts "mcsh" opt; do
    case "$opt" in
    c)
        fmt_c=true
        ;;
    h)
        fmt_h=true
        ;;
    s)
        fmt_sh=true
        ;;
    m)
        fmt_meson=true
        ;;
    *)
        echo "usage: fmt.sh [-amcsH]" >&2
        exit 1
        ;;
    esac
done

if "$fmt_c"; then
    find . -type f -name '*.c' -exec clang-format -i {} +
fi

if "$fmt_h"; then
    find . -type f -name '*.h' -exec clang-format -i {} +
fi

if "$fmt_sh"; then
    find . -type f -name '*.sh' -exec shfmt -w -p {} +
fi

if "$fmt_meson"; then
    find . -type f -name 'meson.*' -exec meson fmt -i {} +
fi
