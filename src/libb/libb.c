#include "syscall.h"
#include <stdarg.h>

#ifndef B_TYPE
    #define B_TYPE long
#endif
#ifndef B_FN
    #define B_FN(name) name
#endif

#define stdin  0
#define stdout 1
#define stderr 2

/* The i-th character of the string is returned */
B_TYPE B_FN(_char)(B_TYPE string, B_TYPE i) __asm__ ("char"); /* alias name */
B_TYPE B_FN(_char)(B_TYPE string, B_TYPE i) {
    return ((char*) string)[i];
}

/* The path name represented by the string becomes the current directory.
   A negative number returned indicates an error. */
B_TYPE B_FN(chdir)(B_TYPE string) {
    return (B_TYPE) syscall(SYS_chdir, string);
}

/* The file specified by the string has its mode changed to the mode argument.
   A negative number returned indicates an error. */
B_TYPE B_FN(chmod)(B_TYPE string, B_TYPE mode) {
    return (B_TYPE) syscall(SYS_chmod, string, mode);
}

/* The file specified by the string has its owner changed to the owner argument.
   A negative number returned indicates an error. */
B_TYPE B_FN(chown)(B_TYPE string, B_TYPE mode) {
    return (B_TYPE) syscall(SYS_chown, string, mode);
}

/* The open file specified by the file argument is closed.
   A negative number returned indicates an error */
B_TYPE B_FN(close)(B_TYPE file) {
    return (B_TYPE) syscall(SYS_close, file);
}

/* The file specified by the string is either truncated or
   created in the mode specified depending on its prior exis-
   tance. In both cases, the file is opened for writing and a
   file descriptor is returned. A negative number returned
   indicates an error.  */
B_TYPE B_FN(creat)(B_TYPE string, B_TYPE mode) {
    return (B_TYPE) syscall(SYS_creat, string, mode);
}

/* The system time (60-ths of a second) represented in the
   two-word vector time is converted to a 16-character date in
   the 8-word v~ctor date. The,,converted date has the follow-
   ing format: "Mmm dd hh:mm:ss" */
void B_FN(ctime)(B_TYPE time, B_TYPE date) {
    // TODO
}
/* The current process is replaced by the execution of the
   file specified by string. The arg-i strings are passed as
   arguments. A return indicates an error. */
void B_FN(execl)(B_TYPE string, ...) {
    // TODO
}

/* The current process is replaced by the execution of the
   file specified by string. The vector of strings of length
   count are passed as arguments. A return indicates an er-
   ror. */
void B_FN(execv)(B_TYPE string, B_TYPE argv, B_TYPE count) {
    syscall(SYS_execve, string, argv, 0);
}

/* The current process is terminated. */
void B_FN(exit)(void) {
    syscall(SYS_exit, 0);
}

/* The current process splits into two. The child process is
   returned a zero. The parent process is returned the pro-
   cess ID of the child. A negative number returned indicates
   an error. */
B_TYPE B_FN(fork)() {
    return (B_TYPE) syscall(SYS_fork);
}

/* The i-node of the open file designated by file is put in
   the 20-word vector status. A negative number returned
   indicates an error. */
B_TYPE B_FN(fstat)(B_TYPE file, B_TYPE status) {
    return (B_TYPE) syscall(SYS_fstat, file, status);
}

/* The next character form the standard input file is re-
   turned. The character ‘*e’ is returned for an end-of-file. */
B_TYPE B_FN(getchar)(void) {
    char c;
    syscall(SYS_read, &c, stdin);
    return c;
}

/* The user-ID of the current process is returned. */
B_TYPE B_FN(getuid)(void) {
    return (B_TYPE) syscall(SYS_getuid);
}

/* The teletype modes of the open file designated by file is
   returned in the 3-word vector tt stat. A negative number .
   returned indicates an error.
   Note: This function does not exist on linux and will always return -1 */
B_TYPE B_FN(gtty)(B_TYPE file, B_TYPE ttystat) {
    (void) file;
    (void) ttystat;
    return -1;
}

/* The character char is stored in the i-th character of the string. */
void B_FN(lchar)(B_TYPE string, B_TYPE i, B_TYPE chr) {
    ((char*) string)[i] = chr;
}

/* The pathname specified by string2 is created such that it
   is a link to the existing file specified by stringl . A
   negative number returned indicates an error. */
B_TYPE B_FN(link)(B_TYPE string1, B_TYPE string2) {
    return (B_TYPE) syscall(SYS_link, string1, string2);
}

/* The directory specified by the string is made to exist with
   the specified access mode. A negative number returned
   indicates an error. */
B_TYPE B_FN(mkdir)(B_TYPE string, B_TYPE mode) {
    return (B_TYPE) syscall(SYS_mkdir, string, mode);
}

B_TYPE B_FN(open)(B_TYPE string, B_TYPE mode) {
    return (B_TYPE) syscall(SYS_open, string, mode);
}

/* predefine some functions */
void B_FN(printn)(B_TYPE n, B_TYPE b);
void B_FN(putchar)(B_TYPE chr);

/* The following function is a general formatting, printing, and
   conversion subroutine. The first argument is a format string.
   Character sequences,of the form ‘%x’ are interpreted and cause
   conversion of type x’ of the next argument, other character
   sequences are printed verbatim. */
void B_FN(printf)(B_TYPE fmt, ...) {
    B_TYPE x, c, i = 0, j;

    va_list ap;
    va_start(ap, fmt);
loop:
    while((c = B_FN(_char)(fmt, i++)) != '%') {
        if(c == '\0')
            goto end;
        B_FN(putchar)(c);
    }
    x = va_arg(ap, B_TYPE);
    switch(c = B_FN(_char)(fmt, i++)) {
        case 'd': /* decimal */
        case 'o': /* octal */
            if(x < 0) {
                x = -x;
                B_FN(putchar)('-');
            }
            B_FN(printn)(x, c == 'o' ? 8 : 10);
            goto loop;
        
        case 'c':
            B_FN(putchar)(x);
            goto loop;
        
        case 's':
            j = 0;
            while((c = B_FN(_char)(x, j++)) != '\0')
                B_FN(putchar)(c);
            goto loop;
    }
    B_FN(putchar)('%');
    i--;
    goto loop;

end:
    va_end(ap);
}

/* The following function will print a non-negative number, n, to
   the base b, where 2<=b<=10, This routine uses the fact that
   in the ANSCII character set, the digits O to 9 have sequential
   code values. */
void B_FN(printn)(B_TYPE n, B_TYPE b) {
    B_TYPE a;
    if((a = n / b))
        B_FN(printn)(a, b);
    B_FN(putchar)(n % b + '0');
}

/* The character char is written on the standard output file. */
void B_FN(putchar)(B_TYPE chr) {
    syscall(SYS_write, &chr, 1);
}

/* Count bytes are read into the vector buffer from the open
   file designated by file. The actual number of bytes read
   are returned. A negative number returned indicates an
   error. */
B_TYPE B_FN(nread)(B_TYPE file, B_TYPE buffer, B_TYPE count) {
    return (B_TYPE) syscall(SYS_read, file, buffer, count);
}

/* The I/O pointer on the open file designated by file is set
   to the value of the designated pointer plus the offset. A
   pointer of zero designates the beginning of the file. A 
   pointer of one designates the current 1/0 pointer. A
   pointer of two designates the end of the file. A negative
   number returned indicates an error. */
B_TYPE B_FN(seek)(B_TYPE file, B_TYPE offset, B_TYPE pointer) {
    return (B_TYPE) syscall(SYS_seek, file, offset, pointer);
}

/* The user-IL) of the current process is set to id. A nega-
   tive number returned indicates an error. */
B_TYPE B_FN(setuid)(B_TYPE id) {
    return (B_TYPE) syscall(SYS_setuid, id);
}

/* The i-node of the file specified by the string is put in
   the 20-word vector status. A negative number returned
   indicates an error. */
B_TYPE B_FN(stat)(B_TYPE string, B_TYPE status) {
    return (B_TYPE) syscall(SYS_stat, string, status);
}

/* The teletype modes of the open file designated by file is
   set from the 3-word vector ttystat. A negative number
   returned indicates an error.
   Note: This function does not exist on linux and will always return -1 */
B_TYPE B_FN(stty)(B_TYPE file, B_TYPE ttystat) {
    (void) file;
    (void) ttystat;
    return -1;
}

/* The current system time is returned in the 2-word vector timev. */
void B_FN(time)(B_TYPE timev) {
    *((long*) timev) = syscall(SYS_time, 0);
}

/* The link specified by the string is removed. A negative
   number returned indicates an error. */
B_TYPE B_FN(unlink)(B_TYPE string) {
    return (B_TYPE) syscall(SYS_unlink, string);
}

/* The current process is suspended until one of its child
   processes terminates. At that time, the child’s process-ID
   is returned. A negative number returned indicates an er-
   ror. */
B_TYPE B_FN(wait)(void) {
    // TODO
    return -1;
}

/* Count bytes are written out of the vector buffer on the
   open file designated by file. The actual number of bytes
   written are returned. A negative number returned indicates
   an error. */
B_TYPE B_FN(nwrite)(B_TYPE file, B_TYPE buffer, B_TYPE count) {
    return (B_TYPE) syscall(SYS_write, file, buffer, count);
}
