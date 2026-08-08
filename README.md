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

> alternatively, view the manpage (`xpt.8`) after installation if documentation was enabled:
> ```sh
> $ man 8 xpt
> ```
> you will need a manpage viewer such as mandoc or man-db.

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

## license

BSD-3-Clause -- see LICENSE

third-party artifacts (libfetch and libfetch/openssl-compat) are specified in THIRD-PARTY-LICENSES


