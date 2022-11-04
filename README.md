# BCause - B compiler for modern systems

**BCause** is a compiler for the **B** programming language, developed by *Ken Thompson* and *Dennis Ritchie* at *Bell Labs* in *1969*, later getting replaced by **C**. BCause is written in C and uses, apart from the C standard library, absolutely zero external dependencies.

This repository also includes a `libb` implementation, B's standard library. It requires zero dependencies, not even libc.

### Current Status

- [x] global variables
- [ ] functions
- [ ] `auto` & `extrn` variables
- [ ] control flow statements
- [ ] expressions

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
To install BCause on your computer globally, use: *(needs root privileges)*
```console
# make install
```

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

[Bell Labs User's Reference to B](https://www.bell-labs.com/usr/dmr/www/kbman.pdf) by Ken Thompson (Jan. 7, 1972)
Wikipedia entry: [B (programming language)](https://en.wikipedia.org/wiki/B_(programming_language))
