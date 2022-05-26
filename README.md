# BCause - B compiler for modern systems

**BCause** is a compiler for the **B** programming language, developed by *Ken Thompson* and *Dennis Ritchie* at *Bell Labs* in *1969*, later getting replaced by **C**. BCause is written in C and uses, apart from the C standard library, absolutely zero external dependencies.

### Current Status

- [ ] token lexing
- [ ] parsing
- [ ] optimization
- [ ] code generation 
- [ ] linking

### Installation

To install BCause, first clone this repository:
```console
$ git clone https://github.com/spydr06/bcause.git
$ cd ./bcause
```
Then, build the project:
```console
$ cmake . && make
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