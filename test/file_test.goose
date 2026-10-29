// Asserts over file.goose: a file read in pieces comes out whole with its
// size known up front, what a path is, and writing a tree. Run from the
// package root: goose run test/file_test.goose
import std;
import os;
import file from "..";

fn main() {
    let path = "file.goose";
    var whole: u8[>..] = [];
    assert(read_file(path, whole));

    let f = file::file_open(path);
    assert(f != 0);
    assert(file::file_size(f) == whole.len);
    var pieces: u8[>..] = [];
    var reads = 0;
    loop {
        let n = file::file_read(f, 100, pieces);
        assert(n >= 0 && n <= 100);
        if n == 0 { break; }
        reads++;
    }
    file::file_close(f);
    assert(pieces == whole);
    assert(reads == (whole.len + 99) / 100);

    assert(file::file_open("no/such/file") == 0);
    assert(file::file_open("src") == 0);       // a directory
    assert(file::file_open("") == 0);

    assert(file::path_kind(path) == file::PATH_FILE);
    assert(file::path_size(path) == whole.len);
    assert(file::path_kind("src") == file::PATH_DIR);
    assert(file::path_size("src") == -1);
    assert(file::path_kind("no/such/file") == -1);
    assert(file::path_kind("") == -1);

    // Writing a tree: a file, its mode, a symbolic and a hard link; all
    // removed again.
    let dir = "test/tree_tmp";
    assert(make_dir(dir));
    assert(write_file("test/tree_tmp/f", "data"));
    assert(file::path_chmod("test/tree_tmp/f", 492));          // 0754 in octal
    assert(file::path_mode("test/tree_tmp/f") == 492);
    assert(file::symlink("f", "test/tree_tmp/s"));
    assert(file::path_kind("test/tree_tmp/s") == file::PATH_LINK);   // not followed
    assert(file::path_size("test/tree_tmp/s") == -1);
    assert(file::hardlink("test/tree_tmp/f", "test/tree_tmp/h"));
    var back: u8[>..] = [];
    assert(read_file("test/tree_tmp/h", back) && back == "data");
    assert(!file::symlink("f", "test/tree_tmp/s"));          // exists already
    assert(file::path_mode("no/such/file") == -1);
    for name in ["s", "h", "f"] { assert(delete_file(str("test/tree_tmp/", name))); }
    assert(delete_dir(dir));
}
