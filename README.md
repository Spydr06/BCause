# BCause - B compiler for modern systems

**BCause** is a compiler for the **B** programming language, developed by *Ken Thompson* and *Dennis Ritchie* at *Bell Labs* in *1969*, later getting replaced by **C**. BCause is written in C99 and relies on a minimal set of dependencies, namely `libc` and the GNU binutils.

This repository also includes a `libb.a` implementation, B's standard library. It requires zero dependencies, not even libc.

BCause is implemented as a small single-pass compiler in ~2000 lines of pure C99 code. Therefore, it features small compile times with a very low memory footprint.

### Current Status

- [x] global variables
- [x] functions
- [x] `auto` & `extrn` variables
- [x] control flow statements
- [x] expressions
- [x] `libb.a` standard library
- [ ] optimization
- [ ] nicer error messages

### Compatibility

Due to BCause's simplicity, only **`gnu-linux-x86_64`**-systems are supported.

- If your system can run *GNU-`make`*, *GNU-`ld`* and *GNU-`as`*, BCause itself should be able to work.
- Because of the reliance on system-calls `libb.a` has to be implemented for each system separately.

> **Note**
> Feel free to submit pull requests to provide more OS support and fix bugs.

### Installation

To install BCause, first clone this repository:
```console
$ git clone https://github.com/spydr06/bcause.git
$ cd ./bcause
```
Then, build the project:
```console
$ make
```
To install BCause on your computer globally, use:
```console
# make install
```
> **Warning**
> this requires root privileges and modifies system files

### Usage

To compile a B source file (`.b`), use:
```console
$ bcause <your file>
```

To get help, type:
```console
$ bcause --help
```

### Licensing
BCause is licensed under the MIT License. See `LICENSE` in this repository for further information.

### References

- [Bell Labs User's Reference to B](https://www.bell-labs.com/usr/dmr/www/kbman.pdf) by Ken Thompson (Jan. 7, 1972)

- Wikipedia entry: [B (programming language)](https://en.wikipedia.org/wiki/B_(programming_language))
