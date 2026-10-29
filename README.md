# clintecker/goose-file

Read a file or stdin in pieces, and ask or change what Goose's `os` module cannot about a path: `lstat`, permission bits, symbolic and hard links. A thin `extern fn` layer over C stdio and POSIX, built from `src/file.c` by the `file.build` sidecar.

Goose's `os` reads a file only whole (`read_file`, `read_stdin`), so a program holds all of its input in memory. With this package it holds one piece at a time:

```goose
import file from "clintecker/goose-file@v1";

// The number of lines in a file, read 64 KB at a time; -1 if it cannot be opened.
fn lines_in(path: const u8[:]) -> i64 {
    let f = file::file_open(path);
    guard f != 0 else { return -1; }
    var buf: u8[>..] = [];
    var lines = 0;
    loop {
        buf.clear();
        let n = file::file_read(f, 65536, buf);
        if n <= 0 { break; }
        for c in buf { if c == '\n' { lines++; } }
    }
    file::file_close(f);
    return lines;
}
```

`test/example.goose` is this example with asserts.

## Functions

| Function | What it does |
|---|---|
| `file_open(path) -> u64` | opens a regular file for reading; 0 if it cannot be opened or is not a regular file |
| `file_stdin() -> u64` | stdin as a handle |
| `file_read(f, n, out) -> i64` | appends up to `n` bytes to the builder `out`; how many, 0 at the end, -1 on an error |
| `file_size(f) -> i64` | the size of an open regular file; -1 for stdin, a pipe or anything else |
| `file_close(f)` | closes a file; stdin stays open |
| `path_kind(path) -> i64` | `PATH_FILE`, `PATH_DIR`, `PATH_LINK` or `PATH_OTHER` for what the path itself is, a symbolic link not followed (`lstat`); -1 if it cannot be read |
| `path_size(path) -> i64` | a regular file's size without opening it; -1 otherwise |
| `path_mode(path) -> i64` | the permission bits of the path itself, or -1 |
| `path_chmod(path, mode) -> bool` | sets the low 12 bits of `mode` as the permission bits |
| `symlink(target, path) -> bool` | makes `path` a symbolic link holding `target` |
| `hardlink(existing, path) -> bool` | makes `path` another name for the file `existing` |

A handle is a C `FILE *` carried as a `u64`; Goose passes it back and never looks inside. Close each handle you open.

## Limits

- POSIX only for the path functions. Windows is not handled: a path reaches `fopen` as bytes, not UTF-16.
- `file_read` appends into a builder; a caller that wants a fixed memory budget clears it between reads, as the example does.
- The C symbols are prefixed `goosefile_`.

## Tests

`test/*.goose` are assert programs, run from this directory with `goose run test/file_test.goose`. They write and remove `test/tree_tmp/`.

MIT licence (LICENSE).
