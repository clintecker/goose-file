/* The C side of file.goose: a file or stdin read in pieces, and
   what a path is and how large, without opening it.
   Goose's os module reads a file only whole; this is the same stdio
   underneath, kept open between calls. A handle is a FILE * crossing into
   Goose as a u64, which Goose passes back and never looks inside. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "goose.h"

/* A const u8[:] as an extern fn hands it over: data and length. */
typedef struct { const uint8_t *data; int64_t len; } goosefile_bytes;

/* `path` as a NUL-terminated name in `name`; 0 if it is empty, holds a NUL
   byte (where C would end it) or is too long for the buffer. */
static int goosefile_name(goosefile_bytes path, char *name, size_t cap) {
    if (path.len <= 0 || path.len >= (int64_t)cap) return 0;
    if (memchr(path.data, 0, (size_t)path.len)) return 0;
    memcpy(name, path.data, (size_t)path.len);
    name[path.len] = 0;
    return 1;
}

/* Opens a regular file for reading; 0 if it cannot be, if the path cannot
   be a C name (goosefile_name), or if it names a directory. */
uint64_t goosefile_file_open(goosefile_bytes path) {
    char name[4096];
    if (!goosefile_name(path, name, sizeof name)) return 0;
    FILE *f = fopen(name, "rb");
    if (!f) return 0;
    struct stat st;
    if (fstat(fileno(f), &st) != 0 || S_ISDIR(st.st_mode)) { fclose(f); return 0; }
    return (uint64_t)(uintptr_t)f;
}

uint64_t goosefile_file_stdin(void) { return (uint64_t)(uintptr_t)stdin; }

/* Appends up to n bytes to `out`; returns how many, 0 at the end, -1 on a
   read error (after appending what came before it). */
int64_t goosefile_file_read(uint64_t h, int64_t n, gs_rref out) {
    FILE *f = (FILE *)(uintptr_t)h;
    uint8_t buf[65536];
    int64_t total = 0;
    while (total < n) {
        size_t want = n - total < (int64_t)sizeof buf ? (size_t)(n - total) : sizeof buf;
        size_t got = fread(buf, 1, want, f);
        gs_bld_append(out, buf, (int64_t)got);
        total += (int64_t)got;
        if (got < want) return ferror(f) ? -1 : total;
    }
    return total;
}

/* The size of a regular file in bytes; -1 for anything else, such as a
   pipe on stdin. */
int64_t goosefile_file_size(uint64_t h) {
    struct stat st;
    if (fstat(fileno((FILE *)(uintptr_t)h), &st) != 0 || !S_ISREG(st.st_mode)) return -1;
    return (int64_t)st.st_size;
}

/* Closes a file; stdin stays open. */
void goosefile_file_close(uint64_t h) {
    FILE *f = (FILE *)(uintptr_t)h;
    if (f && f != stdin) fclose(f);
}

/* What `path` itself is, a symbolic link not followed (lstat): 0 a regular
   file, 1 a directory, 2 a symbolic link, 3 anything else, -1 if it cannot
   be read. */
int64_t goosefile_path_kind(goosefile_bytes path) {
    char name[4096];
    struct stat st;
    if (!goosefile_name(path, name, sizeof name) || lstat(name, &st) != 0) return -1;
    if (S_ISREG(st.st_mode)) return 0;
    if (S_ISDIR(st.st_mode)) return 1;
    if (S_ISLNK(st.st_mode)) return 2;
    return 3;
}

/* The size of the regular file at `path`, without opening it; -1 for
   anything else, a symbolic link included. */
int64_t goosefile_path_size(goosefile_bytes path) {
    char name[4096];
    struct stat st;
    if (!goosefile_name(path, name, sizeof name) || lstat(name, &st) != 0 || !S_ISREG(st.st_mode)) return -1;
    return (int64_t)st.st_size;
}

/* Sets the permission bits of `path` (the low 12 bits of `mode`); 0 on
   failure. */
uint8_t goosefile_path_chmod(goosefile_bytes path, int64_t mode) {
    char name[4096];
    if (!goosefile_name(path, name, sizeof name)) return 0;
    return chmod(name, (mode_t)(mode & 07777)) == 0;
}

/* Makes `path` a symbolic link holding `target`; 0 on failure. */
uint8_t goosefile_symlink(goosefile_bytes target, goosefile_bytes path) {
    char t[4096], name[4096];
    if (!goosefile_name(target, t, sizeof t) || !goosefile_name(path, name, sizeof name)) return 0;
    return symlink(t, name) == 0;
}

/* Makes `path` another name for the existing file `existing`; 0 on
   failure. */
uint8_t goosefile_hardlink(goosefile_bytes existing, goosefile_bytes path) {
    char e[4096], name[4096];
    if (!goosefile_name(existing, e, sizeof e) || !goosefile_name(path, name, sizeof name)) return 0;
    return link(e, name) == 0;
}

/* The permission bits of `path` itself (lstat), or -1. */
int64_t goosefile_path_mode(goosefile_bytes path) {
    char name[4096];
    struct stat st;
    if (!goosefile_name(path, name, sizeof name) || lstat(name, &st) != 0) return -1;
    return (int64_t)(st.st_mode & 07777);
}
