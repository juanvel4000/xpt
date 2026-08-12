# Changelog

All notable changes to xpt will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- quick start section in README.md
- xpt/libxpt guard in meson.build
### Changed
- usage section in README.md
- specify the tools used by scripts/fmt.sh in CHANGELOG.md
### Fixed
- doc/ path in test_ebs.sh
- minor improvements to `libxpt(3)`, `rig(1)`, `xpt(8)` and `xpt.manifest(5)`

## [0.8.0] - transactions, virtual packages and rig
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

## [0.7.0] - fix CLI lock handling, expand tests and dev tooling
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

## [0.6.1] - basic test suite and improved documentation
### Added
- basic test suite (unit tests for manifest parsing and dependency resolver, integration test for ebs formula builds)
- documentation and configuration sections in README
- CHANGELOG.md and ROADMAP.md

## [0.6.0] - merge ebs, expand the documentation
### Added
- ebs build system merged into the xpt codebase
- manpages for ebs(1), ebs(5), ecw(8), xpt.manifest(5) and xpt.tree(5)
- example ebs formula in docs/examples

### Changed
- VERSION file is now the single source of truth for the project version
- meson.build and docs/meson.build formatting cleanup

## [0.5.2] - add `XPT_EX_*` error codes, path traversal protection, new manifest fields and networking-free builds
### Added
- license, homepage and build_epoch manifest fields
- support for building with networking disabled
### Fixed
- path traversal protection
- meaningful exit codes via XPT_EX_*

## [0.5.1] -  add a xpt_print_build_info() function and allow static linking in xpt
### Added
- xpt_print_build_info() in libxpt and a -B command in xpt
- static linking for the xpt binary

## [0.5.0] - separate xpt and libxpt; migrate to meson
### Added
- libxpt as an installable, separate library with its own manpage
- xpt_ prefix on all exposed libxpt functions
### Changed
- migrated the build system from Makefiles to Meson
- use proper sysconfdir/localstatedir paths in libxpt
### Fixed
- treefile is now deleted on installation; empty directories created correctly

## [0.4.2] - sha256 checksum verification and file tracking in the database
### Added
- sha256 checksum verification in package_install_from_repo
- file tracking database

## [0.4.1] - migrate gdbm to sqlite3, create database_printinfo, basic README
### Added
- database_printinfo() in libxpt and a -I command in xpt
- basic README
### Changed
- migrated the local database backend from gdbm to sqlite3

## [0.4.0] - add a lockfile and drop long-options
### Added
- basic lockfile to prevent concurrent runs
- separate xpt/Makefile
### Changed
- dropped long-options, standardized the usage text

## [0.3.2] - organization improvements
### Changed
- dropped xpt-make.sh, cleaned up the Makefile, adopted clang-format

## [0.3.1] - proper formatting, add missing libraries to Makefile
### Fixed
- added `-lssl` and `-lcrypto` to Makefile

## [0.3.0] - repositories, dependency resolving and package downloading
### Added
- basic repo fetching
- basic dependency resolving
- basic package downloading
### Changed
- added `-lz` to LDFLAGS, updated LICENSE contents

## [0.2.0] - list-packages and package-tree, remove directories
### Added
- --list-packages function
- --package-tree function
### Fixed
- empty directories are now removed when uninstalling a package

## [0.1.2] - fix database.c error; overall improvements to xpt-make.sh
### Fixed
- database.c error
### Changed
- overall improvements in xpt-make.sh

## [0.1.1] - dist target in Makefile
### Added
- dist target in Makefile

## [0.1.0] - first functional release
### Added
- versioning support
- uninstall support
- gdbm-based database
- basic package_install function
- basic manifest parsing
- basic .tar.gz extractor
- first functional release of xpt
