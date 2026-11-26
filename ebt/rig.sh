#!/bin/sh

set -eu

# rig (repository index generator): generate xpt.index files from a directory

print_usage() {
    echo "usage: rig [-fhqr] [-d directory] [-u url] [-o output]"
}

url="https://example.com/xpt"
directory="."
quiet="false"
recursive="false"
output="xpt.index"
force="false"

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
    f)
        force=true
        ;;
    h)
        printf "%s\n" 'rig index generator'
        print_usage
        printf "%s\n\n" 'generate xpt.index files from a directory'
        printf "%s\n" 'options:'
        printf " %-10s %s\n" '-d <directory>' 'set a directory to find the xpt packages (default .)'
        printf " %-10s %s\n" '-u <url>' 'specify an url to use as prefix'
        printf " %-10s %s\n" '-o <output>' 'specify an output file'
        printf " %-10s %s\n" '-r' 'enable recursive mode'
        printf " %-10s %s\n" '-q' 'enable quiet mode'
        printf " %-10s %s\n" '-h' 'show this message'
        printf " %-10s %s\n\n" '-f' 'force'

        printf " %s\n\n" 'if -o is set as -, the final index will be printed to stdout.'
        printf "%s\n" "see rig(1) for more information."
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
        if [ "${force:-}" = "true" ]; then
            rm -f "${output}"
        else
            echo "! ${output} already exists"
            exit 1
        fi
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
