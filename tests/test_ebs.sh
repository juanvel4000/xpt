#!/bin/sh
set -eu

EBS="$1"
FORMULA_SRC="$2"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

cp "$FORMULA_SRC" "$WORKDIR/ebs.formula"
cd "$WORKDIR"

pkgname=$(sed -n 's/^pkgname=//p' ebs.formula | tr -d '"'"'"'')
pkgver=$(sed -n 's/^pkgver=//p' ebs.formula | tr -d '"'"'"'')

"$EBS" build ebs.formula

expected="$pkgname-$pkgver.xpt"
if [ ! -f "$WORKDIR/$expected" ]; then
    echo "! expected package $expected was not produced" >&2
    ls -la "$WORKDIR" >&2
    exit 1
fi

if ! tar --zstd -tf "$WORKDIR/$expected" | grep -q '^\./xpt\.manifest$'; then
    echo "! $expected does not contain xpt.manifest" >&2
    exit 1
fi

echo "ebs build smoke test passed: $expected"
