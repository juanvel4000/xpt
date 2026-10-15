# xpt

a package-management suite for `.xpt` packages.

xpt consists of the `libxpt(3)` package-management library, the `xpt(8)` command-line frontend, and the `ebt` build suite.

## features

- install/remove/list packages, dependency resolution, repo sync
- lightweight libfetch implementation derived from NetBSD
- sqlite3-backed local database storage
- simple zstd-based package format
- POSIX `sh`-based post-install triggers
- virtual packages

## quick start

```sh
meson setup build --prefix=/usr --sysconfdir=/etc --localstatedir=/var --buildtype=release # setup the project
meson compile -C build # compile xpt, libxpt
meson test -C build # run the test suite
meson install -C build # install xpt, libxpt, ebt, and documentation
```

## dependencies

build-time dependencies include

- Meson
- Ninja
- libc
- a C compiler
- pkg-config
- SQLite3
- libarchive (with zstd and tar support)
- zstd
- OpenSSL (if networking is enabled)

> libfetch is built as part of the project when networking support is enabled.
>
> dynamically-linked builds require `libarchive`, `sqlite3`, `zstd`, and `openssl` as available libraries

## building

### configuration

xpt exposes several meson build-time options

| option       | type    | default    | description                                                                      |
| ------------ | ------- | ---------- | -------------------------------------------------------------------------------- |
| `static`     | boolean | `false`    | build a statically linked xpt executable, requires static dependencies available |
| `networking` | feature | `enabled`  | build libxpt with `libfetch` networking support                                  |
| `ebt`        | feature | `enabled`  | install the ebt build system                                                     |
| `doc`        | feature | `enabled`  | documentation                                                                    |
| `ebt_only`   | feature | `disabled` | only install ebt                                                                 |
| `tests`      | feature | `enabled`  | enable the test suite                                                            |
| `doxygen`    | feature | `auto`     | generate API documentation with doxygen                                          |

> set with, e.g.:
>
> ```sh
> meson setup build -Dstatic=true -Dnetworking=disabled
> ```

### testing

the `tests/` directory provides a basic test suite for

- simple dependency resolution
- package manifest parsing
- ebs formula builds
- ebs checksum mismatch

these can be executed by running

```sh
meson test -C build
```

### installation

```sh
meson install -C build
```

## usage

xpt uses a unique verb-item argument system, lacking GNU-style long-options completely.

### items

| item        | represents                             |
| ----------- | -------------------------------------- |
| `-f <file>` | a file package                         |
| `-p <pkg>`  | an installed package                   |
| `-n <pkg>`  | a network package                      |
| `-d <dir>`  | a target root directory (default: `/`) |

### verbs

| verb | description                                  | supports items   |
| ---- | -------------------------------------------- | ---------------- |
| `-V` | show the xpt version                         | —                |
| `-i` | install a package                            | `-d`, `-f`, `-n` |
| `-g` | get the version of an installed package      | `-d`             |
| `-h` | show the help message                        | —                |
| `-r` | remove a package                             | `-d`, `-p`       |
| `-l` | list the packages installed in the system    | `-d`             |
| `-t` | show the files used by a package             | `-d`, `-p`       |
| `-s` | download the repository indexes              | `-d`             |
| `-Q` | get information of a package in a repository | `-d`, `-n`       |
| `-I` | show metadata about a package                | `-d`, `-p`       |
| `-B` | print the libxpt build info                  | —                |

### examples

examples use a package called `hello`, as shown in `doc/examples/ebs.formula` and `doc/examples/xpt.manifest`.

#### install a local package

uses `install` (`-i`) and `file package` (`-f`)

```sh
xpt -if hello-0.1.0.xpt
```

#### install a network package

uses `install` (`-i`) and `network package` (`-n`)

```sh
xpt -in hello
```

#### remove an installed package

uses `remove` (`-r`) and `installed package` (`-p`)

```sh
xpt -rp hello
```

## documentation

manpages are installed when documentation is enabled at build time

| manpage           | describes                          |
| ----------------- | ---------------------------------- |
| `xpt(8)`          | the `xpt` cli                      |
| `libxpt(3)`       | the `libxpt` library api           |
| `ebs(1)`          | the ebs build system               |
| `ebs(5)`          | the ebs formula file format        |
| `ecw(8)`          | the ecw chroot wrapper             |
| `mkfhs(1)`        | creates a `hier(7)` directory tree |
| `rig(1)`          | generates an xpt.index             |
| `xpt.manifest(5)` | the xpt.manifest file format       |
| `xpt.tree(5)`     | the xpt.tree file format           |
| `xpt.index(5)`    | the xpt.index file format          |

> view the installed manual pages with `man`
>
> ```sh
> man 8 xpt
> ```

### doxygen

doxygen documentation based on the docblocks defined in `include/xpt`, is built when the `doxygen` binary is available.

by default, it is installed to `PREFIX/doc/xpt`.

## ebt

a build suite for xpt packages.

ebt bundles a set of tools written in POSIX `sh` for building `.xpt` packages from source.

### tools

- `mkfhs(1)`: creates a `hier(7)` directory tree in the specified directory
- `ecw(8)`: a wrapper around `chroot(8)` with better shell detection and proper bind mounting
- `ebs(1)`: a build system for `ebs(5)` formulae, producing zstd-based `.xpt` packages
- `rig(1)`: a `xpt.index(5)` repository index generator

### installation

to install only ebt

```sh
meson setup build -Debt_only=enabled
meson install -C build
```

## known limitations

- offline installs do not resolve transitive dependencies.
- no GPG-signed repositories or packages yet
- package uninstall could use more polish

## project status

xpt is in active development. while functional, it is **not yet production-ready** for real-world usage. known limitations are documented above.

- [CHANGELOG.md](CHANGELOG.md) -- release history
- [CONTRIBUTING.md](CONTRIBUTING.md) -- how to report bugs and send patches

## license

[BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html), see [LICENSE](LICENSE).

> third-party source is specified in [THIRD-PARTY-LICENSES](THIRD-PARTY-LICENSES).
