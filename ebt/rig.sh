#!/bin/sh
# rig: repository index generator
# generate xpt.index files from a directory

set -eu

# shellcheck disable=SC3040
if (set -o pipefail) 2>/dev/null; then
	set -o pipefail
fi

print_usage() {
	echo "usage: rig [-hqr] [-d directory] [-u url] [-o output]"
}

url="https://example.com/xpt"
directory="."
quiet="false"
recursive="false"
output="xpt.index"

safe_print() {
	if [ "${quiet:-false}" = "false" ]; then
		printf "%s\n" "$*"
	fi
}

while getopts "hqru:d:o:" opt; do
	case "$opt" in
	q)
		quiet="true"
		;;
	r)
		recursive="true"
		;;
	u)
		url="${OPTARG}"
		;;
	o)
		output="${OPTARG}"
		;;
	d)
		directory="${OPTARG}"
		;;
	h)
		echo "rig"
		print_usage
		echo ""
		echo "generate xpt.index files from a directory"
		echo ""
		echo "options"
		echo "  -d <directory>  set a directory to find the xpt packages (default .)"
		echo "  -u <url>        specify an url to use as prefix"
		echo "  -o <output>     specify an output file"
		echo "  -r              enable recursive mode"
		echo "  -q              enable quiet mode"
		echo "  -h              show this message"
		echo ""
		echo "if -o is set as -, the final index will be printed to stdout."
		echo "part of ebt build tools (ebt); see rig(1) for more information."
		exit 0
		;;
	*)
		print_usage
		exit 1
		;;
	esac
done

if [ "${output}" = "-" ]; then
	output="/dev/stdout"
else
	if [ -f "${output}" ]; then
		echo "! ${output} already exists"
		exit 1
	fi
	safe_print "saving to ${output}"
fi

pkglist=$(find "${directory:-.}" -type f -name '*.xpt' | awk -v root="${directory:-.}" '
{
    path = $0
    sub("^" root "/?", "", path)
    if (path !~ /\//) print $0
}')
if [ "${recursive:-false}" = "true" ]; then
	pkglist=$(find "${directory:-.}" -type f -name '*.xpt')
fi

if [ -z "${pkglist:-}" ]; then
	echo "! no packages found"
	exit 1
fi

if ! [ -f "${output}" ] && [ "$output" != "/dev/stdout" ]; then
	touch "${output}"
fi

printf "%s\n" "${pkglist}" |
	while IFS= read -r pkg; do
		if ! [ -f "${pkg}" ]; then
			safe_print "warning: ${pkg} not found"
			continue
		fi

		manifest=$(tar -xOf "${pkg}" ./xpt.manifest) || {
			safe_print "warning: could not read manifest from ${pkg}"
			continue
		}

		name=$(printf '%s\n' "${manifest}" | awk -F= '/^name[ \t]*=/ {gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2}')
		version=$(printf '%s\n' "${manifest}" | awk -F= '/^version[ \t]*=/ {gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2}')
		arch=$(printf '%s\n' "${manifest}" | awk -F= '/^arch[ \t]*=/ {gsub(/^[ \t]+|[ \t]+$/, "", $2); print $2}')

		if [ -z "${name}" ] || [ -z "${version}" ] || [ -z "${arch}" ]; then
			safe_print "warning: incomplete manifest in ${pkg}"
			continue
		fi

		depends="$(printf '%s\n' "${manifest}" | awk -F= '/^depends[ \t]*=/ {gsub(/^[ \t]+|[ \t]+$/,"",$2); print $2}')"
		sha256sum="$(sha256sum "${pkg}" | awk '{ print $1}')"
		pkgurl="${url}/${pkg#"$directory"/}"

		safe_print "found ${name}@${version} (${arch})"
		entry="${name}|${version}|${arch}|${depends}|${pkgurl}|${sha256sum}"

		echo "${entry}" >>"${output}"
	done
