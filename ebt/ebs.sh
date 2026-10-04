#!/bin/sh
# ebs build system
# build xpt packages from ebs.formula

set -eu

# shellcheck disable=SC3040
if (set -o pipefail) 2>/dev/null; then
	set -o pipefail
fi

if command -v fakeroot >/dev/null 2>&1; then
	FAKEROOT="fakeroot"
else
	FAKEROOT=""
fi

if command -v aria2c >/dev/null 2>&1; then
	download() {
		aria2c -o "$2" "$1"
	}
	DLPROG="aria2c"
elif command -v wget >/dev/null 2>&1; then
	download() {
		wget -O "$2" "$1"
	}
	DLPROG="wget"
elif command -v curl >/dev/null 2>&1; then
	download() {
		curl -L -o "$2" "$1" --progress-bar
	}
	DLPROG="curl"
else
	echo "! no url transfer program found, tried"
	echo "! aria2c, wget, curl"
	exit 1
fi

if [ "${1:-}" = "" ]; then
	echo "! usage: ebs <command> [arguments]"
	exit 1
fi

print_ebs_help() {
	echo "ebs"
	echo "usage: ebs <command> [arguments]"
	echo ""
	echo "build xpt packages from ebs formulae"
	echo ""
	echo "commands"
	echo "  help    show this message"
	echo "  build   build from an ebs formula"
	echo "  buildt  build from an ebs formula and install to the toolchain specified in \$TOOLCHAIN_DIR"
	echo ""
	echo "part of ebt build tools (ebt)"
	echo "see ebs(1) for more information about the available commands and ebs(5) for the formulae specification"
}

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
	echo "$tree" >"$dir/xpt.tree"
	if [ -n "$tmpd" ] && [ -f "$tmpd" ]; then
		mv "$tmpd" "$dir/xpt.manifest"
	fi
	echo "successfully created a treefile for $dir"
}

extract_source() {
	archive="$1"
	builddir="$2"

	mkdir -p "$builddir"

	case "$archive" in
	*.tar.gz | *.tgz)
		command -v tar >/dev/null 2>&1 || {
			echo "! tar not found"
			exit 1
		}
		tar -xzf "$archive" -C "$builddir"
		;;
	*.tar.xz)
		command -v tar >/dev/null 2>&1 || {
			echo "! tar not found"
			exit 1
		}
		tar -xJf "$archive" -C "$builddir"
		;;
	*.tar.bz2)
		command -v tar >/dev/null 2>&1 || {
			echo "! tar not found"
			exit 1
		}
		tar -xjf "$archive" -C "$builddir"
		;;
	*.tar.zst)
		command -v tar >/dev/null 2>&1 || {
			echo "! tar not found"
			exit 1
		}
		tar --zstd -xf "$archive" -C "$builddir"
		;;
	*.zip)
		command -v unzip >/dev/null 2>&1 || {
			echo "! unzip not found"
			exit 1
		}
		unzip "$archive" -d "$builddir"
		;;
	*)
		echo "! unsupported archive format: $archive"
		exit 1
		;;
	esac
}

build_formula() {
	SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH:-0}
	initdir="$(pwd)"
	formula_script="${1:-ebs.formula}"

	case "$formula_script" in
	/*) formula_path="$formula_script" ;;
	*) formula_path="$initdir/$formula_script" ;;
	esac

	if ! [ -f "$formula_path" ]; then
		echo "! formula $formula_path does not exist"
		exit 1
	fi
	# shellcheck disable=SC1090
	. "$formula_path"
	[ -n "$ebsver" ] || {
		echo "! ebsver not set in $formula_script"
		exit 1
	}

	case "$ebsver" in
	1) ;;
	*)
		echo "! incompatible ebs formula version"
		exit 1
		;;
	esac

	echo "* validating formula"

	type build >/dev/null 2>&1 || {
		echo "! function 'build()' not found"
		exit 1
	}

	type package >/dev/null 2>&1 || {
		echo "! function 'package()' not found"
		exit 1
	}
	[ -n "$pkgname" ] || {
		echo "! attribute 'pkgname=' not set in $formula_script"
		exit 1
	}
	[ -n "$pkgver" ] || {
		echo "! attribute 'pkgver=' not set in $formula_script"
		exit 1
	}
	[ -n "$pkgmaintainer" ] || {
		echo "! attribute 'pkgmaintainer=' not set in $formula_script "
		exit 1
	}
	[ -n "$pkgdesc" ] || {
		echo "! attribute 'pkgdesc=' not set in $formula_script"
		exit 1
	}

	echo "* formula is valid"

	pkgdir=$(mktemp -d)
	if ! [ -d "$pkgdir" ]; then
		echo "! $pkgdir does not exist"
		exit 1
	fi
	trap 'rm -rf "$pkgdir"' EXIT

	buildroot=$(mktemp -d)
	trap 'rm -rf "$buildroot" "$pkgdir"' EXIT

	if [ -n "${pkgsrc:-}" ]; then
		[ -n "$pkgchecksum" ] || {
			echo "! attribute 'pkgchecksum=' not set in $formula_script"
			exit 1
		}
		echo "* downloading $pkgsrc"
		srcfile=$(basename "${pkgsrc%%\?*}")
		download "$pkgsrc" "$srcfile" || {
			echo "! $DLPROG failed"
			exit 1
		}
		echo "* verifying source"

		algo="${pkgchecksum%%:*}"
		sum="${pkgchecksum#*:}"
		case "$algo" in
		sha256) cmd=sha256sum ;;
		sha512) cmd=sha512sum ;;
		*)
			echo "! unknown checksum algorithm: $algo"
			exit 1
			;;
		esac

		printf '%s  %s/%s\n' "$sum" "$initdir" "$srcfile" | "$cmd" -c - || {
			echo "! checksum mismatch"
			exit 1
		}

		echo "* extracting source"
		extract_source "$initdir/$srcfile" "$buildroot"
		trap 'rm -rf "$initdir/$srcfile" "$buildroot" "$pkgdir"' EXIT
	fi

	cd "$buildroot"

	set -- ./*
	if [ "$#" -eq 1 ] && [ -d "$1" ]; then
		cd "$1"
	fi

	if type prepare >/dev/null 2>&1; then
		echo "* preparing the package | prepare()"
		prepare || {
			echo "! failed preparing the package"
			exit 1
		}
	fi

	echo "* building $pkgname@$pkgver | build()"
	build || {
		echo "! failed building the package"
		exit 1
	}

	if [ "${pkgcheck:-true}" = "true" ] && type check >/dev/null 2>&1; then
		echo "* running checks | check()"
		check || {
			echo "! check() failed"
			exit 1
		}
	fi

	echo "* packaging $pkgname | package()"
	package "$FAKEROOT" || {
		echo "! error packaging the package"
		exit 1
	}

	createTreefile "$pkgdir"

	echo "* compressing the package"
	output_file=${2:-"$pkgname-$pkgver.xpt"}
	if [ "$output_file" = "ebs.auto" ]; then
		output_file="$pkgname-$pkgver.xpt"
	fi
	cat <<EOF >"$pkgdir/xpt.manifest"
// generated by ebs
build_epoch=${SOURCE_DATE_EPOCH:-0}

name=$pkgname
version=$pkgver
arch=${pkgarch:-$(uname -m)}
desc=$pkgdesc
maintainer=$pkgmaintainer
depends=${pkgdeps:-}
provides=${pkgprovides:-}
EOF

	if [ "${pkglicense:-""}" != "" ]; then
		echo "license=${pkglicense}" >>"$pkgdir/xpt.manifest"
	fi

	if [ "${pkghomepage:-""}" != "" ]; then
		echo "homepage=${pkghomepage}" >>"$pkgdir/xpt.manifest"
	fi

	if [ "${3:-}" = 'install_to_toolchain' ]; then
		echo "* attempting to install to toolchain"
		if ! [ -n "$TOOLCHAIN_DIR" ]; then
			echo "! toolchain dir not found"
			exit 1
		fi
		command -v install_to_toolchain >/dev/null 2>&1 || {
			echo "! install_to_toolchain not found in $formula_script"
			exit 1
		}
		install_to_toolchain "$TOOLCHAIN_DIR" "$FAKEROOT"
		echo "* installed to toolchain at $TOOLCHAIN_DIR"
	fi

	if ! tar --zstd --sort=name --mtime="@${SOURCE_DATE_EPOCH}" --owner=0 --group=0 --numeric-owner -cf "$initdir/$output_file" -C "$pkgdir" .; then
		echo "! failed to create archive"
		exit 1
	fi
	echo "* $initdir/$output_file is ready."
}

case "${1:-}" in
help | -h | h)
	print_ebs_help
	exit 0
	;;

build | -b | b)
	build_formula "${2:-./ebs.formula}" "${3:-ebs.auto}"
	;;

buildt | -t | t)
	build_formula "${2:-./ebs.formula}" "${3:-ebs.auto}" "install_to_toolchain"
	;;

*)
	echo "ebs: invalid option -- '$1'"
	exit 1
	;;
esac
