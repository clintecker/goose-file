// The README's example, run by make pkg-test so it keeps compiling: count
// the lines of a file 64 KB at a time, holding no more than that.
import std;
import os;
import file from "..";

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

fn main() {
    var whole: u8[>..] = [];
    assert(read_file("file.goose", whole));
    var want = 0;
    for c in whole { if c == '\n' { want++; } }
    assert(lines_in("file.goose") == want);
    assert(lines_in("no/such/file") == -1);
}
