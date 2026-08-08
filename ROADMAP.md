# roadmap

this document tracks known limitations and planned work for xpt.

## known limitations

these are real, understood gaps -- not surprises waiting to be found.

- **offline/local installs cannot resolve transitive dependencies.**

- **checksum mismatch handling is untested.** the code path exists in `ebs(1)` but has no test coverage yet.

## planned

in rough priority order

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
- [X] transitive dependency resolution
