// Reading a file or stdin in pieces, and what os cannot say or do about a
// path. Goose's os module reads a file only whole (read_file, read_stdin),
// so a program holds its whole input in memory; these keep a C stdio stream
// open between reads instead (src/file.c, named in file.build).
namespace file;

// A handle is an open stream as a u64; 0 is "could not open".
extern "goosefile_file_open" fn file_open(path: const u8[:]) -> u64;   // a regular file; 0 on failure
extern "goosefile_file_stdin" fn file_stdin() -> u64;
extern "goosefile_file_read" fn file_read(f: u64, n: i64, out: u8[>..]&) -> i64;  // appends up to n; 0 at end, -1 on error
extern "goosefile_file_size" fn file_size(f: u64) -> i64;             // -1 if not a regular file
extern "goosefile_file_close" fn file_close(f: u64);                  // leaves stdin open

// What a path itself is, a symbolic link not followed (lstat), and a regular
// file's size without opening it. os can tell only "a file" from "a
// directory", and follows links.
let PATH_FILE = 0;
let PATH_DIR = 1;
let PATH_LINK = 2;
let PATH_OTHER = 3;
extern "goosefile_path_kind" fn path_kind(path: const u8[:]) -> i64;  // PATH_*, or -1 if it cannot be read
extern "goosefile_path_size" fn path_size(path: const u8[:]) -> i64;  // -1 unless a regular file

// What os cannot do when writing a tree: set permission bits, and make
// symbolic and hard links. Each returns false on failure.
extern "goosefile_path_chmod" fn path_chmod(path: const u8[:], mode: i64) -> bool;
extern "goosefile_symlink" fn symlink(target: const u8[:], path: const u8[:]) -> bool;
extern "goosefile_hardlink" fn hardlink(existing: const u8[:], path: const u8[:]) -> bool;
extern "goosefile_path_mode" fn path_mode(path: const u8[:]) -> i64;  // permission bits, or -1
