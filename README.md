 # xpt

a minimalistic, lightweight package manager for `.xpt` packages, written in C.

xpt consists of the `libxpt(3)` package-management library and the `xpt(8)` command-line frontend. the repository also contains ebt, a separate set of POSIX shell tools for building `.xpt` packages.

## features

- install/remove/list packages, dependency resolution, repo sync
- lightweight libfetch implementation derived from NetBSD
- sqlite3-backed local database storage
- simple zstd-based package format

## quick start

setup the project

```sh
 $ meson setup build --sysconfdir=/etc --localstatedir=/var
```

> `--localstatedir=/var` sets `/var` as the LOCALSTATEDIR, instead of the default `/var/local`
> `--sysconfdir=/etc` sets `/etc` as the SYSCONFDIR, instead of the default `PREFIX/etc`
> you might also want to set `--prefix=/usr` to set PREFIX as `/usr` instead of the default `/usr/local`
> you might want to set `--buildtype=release` to disable any debug material
> view `meson configure` for more options, or the [`building / configuration`](#building) section in this README

build the project

```
 $ meson compile -C build
```

> optionally, run tests
> ```
>  $ meson test -C build
> ```

install the project

```sh
 $ meson install -C build
```

> **note**: this command assumes the user has write-access in the directory specified by DESTDIR (default: `/`)

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
> if built as a shared binary, xpt will require the previous as runtime dependencies.

## building

### configuration

xpt exposes several meson build-time options

| option       | type       | default    | description                              |
|--------------|------------|------------|------------------------------------------|
| `static`     | boolean    | `false`    | build a statically linked xpt executable |
| `networking` | feature    | `enabled`  | build xpt with networking support        |
| `ebt`        | feature    | `enabled`  | ebt build system                         |
| `doc`        | feature    | `enabled`  | xpt/ebt documentation                    |
| `ebt_only`   | feature    | `disabled` | only build ebt                           |
| `tests`      | feature    | `enabled`  | run the xpt tests suite                  |

> set with, e.g.:
> ```sh
>  $ meson setup build -Dstatic=true -Dnetworking=disabled
> ```

### testing

the `tests/` directory provides a simple test suite for

- simple dependency resolution
- package manifest parsing
- ebs formula builds
- ebs checksum mismatch

these can be executed by running

```sh
 $ meson test -C build
```

### installation

```sh
  $ meson install -C build
```

## usage

xpt uses a unique verb-item argument system

### items

| item        | description                          |
|-------------|--------------------------------------|
| `-f <file>` | file package                         |
| `-p <pkg>`  | installed package                    |
| `-n <pkg>`  | network package                      |
| `-d <dir>`  | destination directory (default: `/`) |

### verbs

| verb | description                                  | usable items     |
|------|----------------------------------------------|------------------|
| `-V` | show the current xpt version                 | --               |
| `-i` | install a package                            | `-d`, `-f`, `-n` |
| `-g` | get the version of an installed package      | `-d`             |
| `-h` | show the help message                        | --               |
| `-r` | remove a package                             | `-d`, `-p`       |
| `-l` | list the packages installed in the system    | `-d`             |
| `-t` | show the files used by a package             | `-d`, `-p`       |
| `-s` | download the repository indexes              | `-d`             |
| `-Q` | get information of a package in a repository | `-d`, `-n`       |
| `-I` | show metadata about a package                | `-d`, `-p`       |
| `-B` | print the libxpt build info                  | --               |

### examples

examples use a package called `hello`, as shown in `doc/examples/ebs.formula` and `doc/examples/xpt.manifest`

#### install a local package

uses `install` (`-i`) and `file package` (`-f`)

```sh
 $ xpt -if hello-0.1.0.xpt
```

#### install a network package

uses `install` (`-i`) and `network package` (`-n`)

```sh
 $ xpt -in hello
```

#### remove an installed package

uses `remove` (`-r`) and `installed package` (`-p`)

```sh
 $ xpt -rp hello
```


## documentation

manpages are installed when documentation is enabled at build time

| manpage           | describes                          |
|-------------------|------------------------------------|
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

> view any of them with, e.g.:
> ```sh
>  $ man 8 xpt
> ```
> you will need a manpage viewer such as mandoc or man-db
## ebt

a lightweight package build toolkit.

ebt bundles a set of tools written in POSIX `sh` for building `.xpt` packages from source.

### tools

- `mkfhs(1)`: creates a `hier(7)` directory tree
- `ecw(8)`: a wrapper around `chroot(8)`
- `ebs(1)`: a build system for `ebs(5)` formulae, producing zstd-based `.xpt` packages
- `rig(1)`: a `xpt.index(5)` repository index generator

### installation

to install only ebt

```sh
  $ meson setup build -Debt_only=enabled
  $ meson install -C build
```

## limitations

- offline / installs do not resolve transitive dependencies.
- no GPG-signed repositories or packages yet
- package uninstall could use more polish

## project status

- [CHANGELOG.md](CHANGELOG.md) -- release history
- [CONTRIBUTING.md](CONTRIBUTING.md) -- how to report bugs and send patches

## license

BSD-3-Clause -- see LICENSE

third-party source is specified in THIRD-PARTY-LICENSES


