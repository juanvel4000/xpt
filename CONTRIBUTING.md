# contributing to xpt

xpt is maintained on [sourcehut](https://sr.ht/~juanvel400/xpt) -- this document explains how to report bugs and send patches there.

## reporting bugs / requesting features

use the [ticket tracker](https://todo.sr.ht/~juanvel400/xpt). when filing a bug include

- your OS and xpt version (`xpt -V`)
- steps to reproduce
- what you expected vs. what happened

## sending patches

xpt doesn't use GitHub-style pull requests. instead:

1. clone the repo and make your changes on a branch
2. commit with a clear, scoped message (see "commit style" below)
3. configure `git send-email` if you haven't already -- see
[sr.ht's send-email tutorial](https://git-send-email.io/)
4. send your patch to: **~juanvel400/xpt-devel@lists.sr.ht**
```sh
 $ git send-email --to=~juanvel400/xpt-devel@lists.sr.ht origin/main..HEAD
```

## commit style

- one logical change per commit
- imperative, lowercase summary line (`add X`, not `Added X` or `adds X`) -- matches the existing log
- keep the summary under ~72 chars; use the body for anything that needs more explanation

## before submitting

- run the test suite: `meson test -C build`
- if you are changing libxpt/xpt, make sure `meson compile -C build` is clean with no warnings
- if your change affects behavior, consider whether it needs a test (see `tests/`) or a CHANGELOG.md entry under `[Unreleased]`

## code style

- shell scripts are POSIX sh -- avoid bashisms
- see existing code for naming conventions (`xpt_`-prefixed public functions in libxpt, etc.)

> `scripts/fmt.sh` formats all of the above in one pass
> ```sh
>  $ scripts/fmt.sh -a
> ```
> `scripts/fmt.sh` uses `meson fmt` for meson files (`-m`), `clang-format` for C sources (including headers) (`-c` and `-h`)
> and `shfmt` for shell scripts (`-s`).

## license

by contributing, you agree your changes are licensed under BSD-3-Clause, matching the rest of the project (see LICENSE).
