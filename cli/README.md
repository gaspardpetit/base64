# base64 command-line utility

This directory builds a standalone, streaming Base64 command compatible with
the GNU coreutils `base64` interface. It uses the GaspardPetit Base64 codec from
this repository and does not build the benchmark suite or GoogleTest.

## Build and test

```sh
cmake -S cli -B build/cli -DCMAKE_BUILD_TYPE=Release
cmake --build build/cli --config Release
ctest --test-dir build/cli -C Release --output-on-failure
```

The streaming buffer defaults to 49,152 bytes. It can be varied for profiling
with `-DBASE64_CLI_BUFFER_SIZE=N`; `N` must be positive and divisible by 12.

The executable supports `-d`/`--decode`, `-i`/`--ignore-garbage`,
`-w`/`--wrap`, `--help`, and `--version`. It reads one file or standard input
and writes to standard output.

For an optional differential check, provide a GNU coreutils executable:

```sh
python cli/tests/oracle.py build/cli/base64 /usr/bin/base64
```

Under MSYS2, invoke the script from its shell and pass the Windows executable
path in MSYS notation.

## Release artifacts

The `Base64 CLI` GitHub Actions workflow builds and tests the command on Linux,
Windows, and macOS. Every run publishes downloadable workflow artifacts; a tag
whose name starts with `v` also creates a GitHub release containing the three
platform archives.
