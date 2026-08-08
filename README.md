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

### installation

```sh
  $ meson install -C build
```

## usage

run

```sh
 $ xpt -h
```

> or view the manpage (`xpt.8`) by running this after install (if you enabled documentation)
> ```sh
> $ man 8 xpt
> ```
> you will need a manpage viewer such as mandoc or man-db.

## ebt

a lightweight package build suite.

ebt bundles a set of tools written in POSIX sh script designed to 

## license

BSD-3-Clause -- see LICENSE

third-party artifacts (libfetch and libfetch/openssl-compat) are specified in THIRD-PARTY-LICENSES


