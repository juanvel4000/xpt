#!/bin/sh
set -eu

EBS="$1"
FORMULA_SRC="$2"

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

cp "$FORMULA_SRC" "$WORKDIR/ebs.formula"
cd "$WORKDIR"

if "$EBS" build ebs.formula > out.log 2>&1; then
    echo "! expected ebs.sh to fail on checksum mismatch." >&2
    cat err.log >&2
    exit 1
fi


if ! grep -q "checksum mismatch" out.log; then
    echo "! expected 'checksum mismatch' in stderr, got:" >&2
    cat err.log >&2
    exit 1
fi

echo "ebs checksum mismatch test passed"
