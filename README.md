# xpt

a minimalistic package manager for `.xpt` packages, written in C.

## features

- install/remove/list packages, dependency resolution, repo sync
- lightweight libfetch implementation (from OpenBSD)
- sqlite3-backed local database storage
- barebones package format

## building

```sh
 $ meson setup build
 $ meson compile -C build
```

### configuration

xpt exposes several meson build-time options

| option       | type       | default    | description                              |
|--------------|------------|------------|------------------------------------------|
| `static`     | boolean    | `false`    | build a statically linked xpt executable |
| `networking` | feature    | `enabled`  | build xpt with networking support        |
| `ebt`        | feature    | `enabled`  | ebt build system                         |
| `docs`       | feature    | `enabled`  | xpt/ebt documentation                    |
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

these can be executed by running

```sh
 $ meson test -C build
```

### installation

```sh
  $ meson install -C build
```

## usage

run

```sh
 $ xpt -h
```

## documentation

manpages are installed alongside the executables if documentation was enabled at build time

| manpage           | describes                    |
|-------------------|------------------------------|
| `xpt(8)`          | the `xpt` cli                |
| `libxpt(3)`       | the `libxpt` library api     |
| `ebs(1)`          | the ebs build system         |
| `ebs(5)`          | the ebs formula file format  |
| `ecw(8)`          | the ecw chroot wrapper       |
| `mkfhs(1)`        | the FHS tree creator         |
| `xpt.manifest(5)` | the xpt.manifest file format |
| `xpt.tree(5)`     | the xpt.tree file format     |

> view any of them with, e.g.:
> ```sh
>  $ man 8 xpt
> ```
> you will need a manpage viewer such as mandoc or man-db
## ebt

a lightweight package build toolkit.

ebt bundles a set of tools written in POSIX `sh` for building `.xpt` packages from source.

### tools

- mkfhs: creates a fully compliant FHS directory tree
- ecw: a wrapper around `chroot(1)`
- ebs: a build system for EBS formula, which outputs a zstd-based `.xpt` package

### installation

to install only ebt

```sh
  $ meson setup build -Debt_only=enabled
  $ meson install -C build
```

## project status

- [CHANGELOG.md](CHANGELOG.md) -- release history
- [ROADMAP.md](ROADMAP.md) -- known limitations and planned work
- [CONTRIBUTING.md](CONTRIBUTING.md) -- how to report bugs and send patches

## license

BSD-3-Clause -- see LICENSE

third-party artifacts (libfetch and libfetch/openssl-compat) are specified in THIRD-PARTY-LICENSES


