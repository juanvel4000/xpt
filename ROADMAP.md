# roadmap

this document tracks known limitations and planned work for xpt.

## known limitations

these are real, understood gaps -- not surprises waiting to be found.

- **dependency resolution is not transitive.** installing a package only solves its direct dependencies. see [`resolve_package()`](libxpt/resolver.c)

- **checksum mismatch handling is untested.** the code path exists in `ebs(1)` but has no test coverage yet.

## planned

in rough priority order

- [ ] transitive dependency resolution
- [ ] test coverage for checksum verification failures
- [ ] test coverage for malformed/incomplete manifests
- [ ] package uninstall improvements
- [ ] GPG-signed repositories, signed packages

## not planned

things intentionally out of scope

- Windows support

## done

recently closed items worth noting here before they migrate to CHANGELOG.md

- [X] basic test suite
