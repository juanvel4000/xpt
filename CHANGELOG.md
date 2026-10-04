# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- format all c source files and headers
- properly format and standarize the ebt scripts
- quick start section in README.md
- xpt/libxpt guard in meson.build

### Changed

- usage section in README.md
- specify the tools used by scripts/fmt.sh in CONTRIBUTING.md

### Fixed

- formatting in libxpt/repo.c and xpt/main.c
- doc/ path in test_ebs.sh
- minor improvements to `libxpt(3)`, `rig(1)`, `xpt(8)` and `xpt.manifest(5)`

## [0.8.0] - 2026-08-09

### Added

- package-file ownership awareness in install/uninstall
- virtual packages / capability system (`provides=`)
- "known limitations" section in README.md
- transactions in the database
- xpt.index(5) manpage
- examples for xpt.index and xpt.manifest
- ebt rig: repository index generator script
- xpt_repos_getpkginfo() in libxpt and a -Q command in xpt

### Changed

- rename docs -> doc
- expand and fix issues in the xpt(8) help text
- add modular options to meson.build

### Removed

- ROADMAP.md

## [0.7.0] - 2026-08-08

### Added

- scripts/fmt.sh script for code formatting
- CONTRIBUTING.md
- unit test for invalid/non-existent manifests
- integration test for checksum mismatch (ebs)

### Changed

- use dl.juanvel400.com/hello example package instead of GNU hello
- format various source files
- improve consistency in README.md and manpages
- fix shellcheck warnings in ebt
- improve the CLI UX; use the lock only in mutable actions

### Removed

- openssl-compat from libfetch

## [0.6.1] - 2026-08-08

### Added

- basic test suite (unit tests for manifest parsing and dependency resolver, integration test for ebs formula builds)
- documentation and configuration sections in README
- CHANGELOG.md and ROADMAP.md

## [0.6.0] - 2026-08-07

### Added

- ebs build system merged into the xpt codebase
- manpages for ebs(1), ebs(5), ecw(8), xpt.manifest(5) and xpt.tree(5)
- example ebs formula in docs/examples

### Changed

- VERSION file is now the single source of truth for the project version
- meson.build and docs/meson.build formatting cleanup

## [0.5.2] - 2026-08-07

### Added

- license, homepage and build_epoch manifest fields
- support for building with networking disabled

### Fixed

- path traversal protection
- meaningful exit codes via XPT_EX_*

## [0.5.1] - 2026-08-06

### Added

- xpt_print_build_info() in libxpt and a -B command in xpt
- static linking for the xpt binary

## [0.5.0] - 2026-08-06

### Added

- libxpt as an installable, separate library with its own manpage
- xpt_ prefix on all exposed libxpt functions

### Changed

- migrated the build system from Makefiles to Meson
- use proper sysconfdir/localstatedir paths in libxpt

### Fixed

- treefile is now deleted on installation; empty directories created correctly

## [0.4.2] - 2026-08-04

### Added

- sha256 checksum verification in package_install_from_repo
- file tracking database

## [0.4.1] - 2026-08-03

### Added

- database_printinfo() in libxpt and a -I command in xpt
- basic README

### Changed

- migrated the local database backend from gdbm to sqlite3

## [0.4.0] - 2026-08-03

### Added

- basic lockfile to prevent concurrent runs
- separate xpt/Makefile

### Changed

- dropped long-options, standardized the usage text

## [0.3.2] - organization improvements

### Changed

- dropped xpt-make.sh cleaned up the Makefile, adopted clang-format

## [0.3.1] - 2026-07-25

### Fixed

- added `-lssl` and `-lcrypto` to Makefile

## [0.3.0] - 2026-07-25

### Added

- basic repo fetching
- basic dependency resolving
- basic package downloading

### Changed

- added `-lz` to LDFLAGS, updated LICENSE contents

## [0.2.0] - 2026-07-23

### Added

- --list-packages function
- --package-tree function

### Fixed

- empty directories are now removed when uninstalling a package

## [0.1.2] - 2025-10-09

### Fixed

- database.c error

### Changed

- overall improvements in xpt-make.sh

## [0.1.1] - 2025-10-09

### Added

- dist target in Makefile

## [0.1.0] - 2025-10-09

### Added

- versioning support
- uninstall support
- gdbm-based database
- basic package_install function
- basic manifest parsing
- basic .tar.gz extractor
- first functional release of xpt
