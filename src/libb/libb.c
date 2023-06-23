// 
// This is a minimal implementation of libb, the standard library for the B programming language (1969)
//

#ifndef B_TYPE
    /* type representing B's single data type (64-bit int on x86_64) */
    #define B_TYPE long long
#endif
#ifndef B_FN
    /* this macro allows to give each B std function a pre- or postfix
       to avoid issues with common names
       e.g.: #define B_FN(name) __b##name <-- gives each B funcion the prefix "__b" */
    #define B_FN(name) name
#endif

// defines the argument buffer size of execl()
#ifndef MAX_EXECL_ARGS
    #define MAX_EXECL_ARGS 128
#endif

// NULL pointer definition
#define NULL ((void*) 0)

// standard unix IO files
#define stdin  0
#define stdout 1
#define stderr 2

/*
VAList wrapper implementation
*/

/* VA aliases */
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
#define va_list __builtin_va_list

/*
Syscall wrapper implementation
*/

/* type used for syscalls */
#define SYSCALL_TYPE long

static inline SYSCALL_TYPE __syscall_ret(unsigned SYSCALL_TYPE r)
{
	if (r > -4096UL) {
		return -1;
	}
	return r;
}

static inline SYSCALL_TYPE __syscall0(SYSCALL_TYPE n)
{
	unsigned SYSCALL_TYPE ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
	return ret;
}

static inline SYSCALL_TYPE __syscall1(SYSCALL_TYPE n, SYSCALL_TYPE a1)
{
	unsigned SYSCALL_TYPE ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
	return ret;
}

static inline SYSCALL_TYPE __syscall2(SYSCALL_TYPE n, SYSCALL_TYPE a1, SYSCALL_TYPE a2)
{
	unsigned SYSCALL_TYPE ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
						  : "rcx", "r11", "memory");
	return ret;
}

static inline SYSCALL_TYPE __syscall3(SYSCALL_TYPE n, SYSCALL_TYPE a1, SYSCALL_TYPE a2, SYSCALL_TYPE a3)
{
	unsigned SYSCALL_TYPE ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3) : "rcx", "r11", "memory");
	return ret;
}

#define __scc(X) ((SYSCALL_TYPE) (X))

#define __syscall1(n,a) __syscall1(n,__scc(a))
#define __syscall2(n,a,b) __syscall2(n,__scc(a),__scc(b))
#define __syscall3(n,a,b,c) __syscall3(n,__scc(a),__scc(b),__scc(c))

#define __SYSCALL_NARGS_X(a,b,c,d,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __syscall(...) __SYSCALL_DISP(__syscall,__VA_ARGS__)
#define syscall(...) __syscall_ret(__syscall(__VA_ARGS__))

/* syscall ids */
#define SYS_read 0
#define SYS_write 1
#define SYS_open 2
#define SYS_close 3
#define SYS_stat 4
#define SYS_fstat 5
#define SYS_seek 8
#define SYS_fork 57
#define SYS_execve 59
#define SYS_exit 60
#define SYS_wait4 61
#define SYS_chdir 80
#define SYS_mkdir 83
#define SYS_creat 85
#define SYS_link 86
#define SYS_unlink 87
#define SYS_chmod 90
#define SYS_chown 92
#define SYS_gettimeofday 96
#define SYS_getuid 102
#define SYS_setuid 105
#define SYS_time 201

/*
assert implementation
*/

#define assert(condition) ((void)((condition) || (__assert_fail(#condition, __FILE__, __LINE__, __func__), 0)))
#define write_str(string) (syscall(SYS_write, 1, (string), strlen((string))))

void B_FN(printn)(B_TYPE n, B_TYPE b);

static long strlen(const char* string)
{
    long i = 0;
    while(string[i++]);
    return i - 1;
}

static void __assert_fail(const char* condition, const char* file, const int line, const char* func)
{
    write_str("Assertion failed: ");
    write_str(condition);
    write_str(" (");
    write_str(file);
    write_str(": ");
    write_str(func);
    write_str(": ");
    B_FN(printn)(line, 10);
    write_str(")\n");

    syscall(SYS_exit, 127);
}

/*
B standard library implementation
*/

/* The `main` function must be declared in any B program */
extern B_TYPE B_FN(main)(void);
void B_FN(exit)(void);

/* entry point of any B program */
void _start(void) __asm__ ("_start"); /* assure, that _start is really named _start in asm */
void _start(void) {
    assert(sizeof(B_TYPE) == sizeof(void*)); /* assert that the size of the B type is equal 
                                                to the word (address) size. This is crucial
                                                for any B program to work correctly.*/
    B_TYPE code = B_FN(main)();
    syscall(SYS_exit, code);
}

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
   the 8-word vector date. The converted date has the follow-
   ing format: "Mmm dd hh:mm:ss" */
static char month_strs[12][3] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void B_FN(ctime)(B_TYPE time_vec, B_TYPE date) {
    short hour, minute, second;
    long a, b, c, d, month, day;
    B_TYPE time = *(B_TYPE*) time_vec;
    char *date_vec = (void*) date;

    second = time % 60;
    time /= 60;
    minute = time % 60;
    time /= 60;
    hour = time % 24;
    time /= 24;

    a = (4 * time + 102032) / 146097 + 15;
    b = time + 2442113 + a - a / 4;
    c = (20 * b - 2442) / 7305;
    d = b - 365 * c - c / 4;
    month = d * 1000 / 30601;
    day = d - month * 30 - month * 601 / 1000;

    if(month <= 13) {
        c -= 4716;
        month -= 2;
    }
    else {
        c -= 4715;
        month -= 14;
    }

    date_vec[0] = month_strs[month][0];
    date_vec[1] = month_strs[month][1];
    date_vec[3] = month_strs[month][2];
    date_vec[4] = ' ';
    date_vec[5] = day / 10 + '0';
    date_vec[6] = day % 10 + '0';
    date_vec[7] = ' ';
    date_vec[8] = hour / 10 + '0';
    date_vec[9] = hour % 10 + '0';
    date_vec[10] = ':';
    date_vec[11] = minute / 10 + '0';
    date_vec[12] = minute % 10 + '0';
    date_vec[13] = ':';
    date_vec[14] = second / 10 + '0';
    date_vec[15] = second % 10 + '0';
}

/* The current process is replaced by the execution of the
   file specified by string. The arg-i strings are passed as
   arguments. A return indicates an error. */
void B_FN(execl)(B_TYPE string, ...) {
    static B_TYPE args[MAX_EXECL_ARGS];
    B_TYPE envp;
    int i = 0;

    va_list ap;
    va_start(ap, string);

    while((args[i] = va_arg(ap, B_TYPE)) && i <= MAX_EXECL_ARGS);

    syscall(SYS_execve, string, args, &envp);
    va_end(ap);
}

/* The current process is replaced by the execution of the
   file specified by string. The vector of strings of length
   count are passed as arguments. A return indicates an er-
   ror. */
void B_FN(execv)(B_TYPE string, B_TYPE argv, B_TYPE count) {
    B_TYPE envp = 0;
    B_TYPE args[count + 1];
    for(B_TYPE i = 0; i < count; i++) {
        args[i] = ((B_TYPE*)argv)[i];
    }
    args[count] = 0;
    syscall(SYS_execve, string, args, &envp);
}

/* The current process is terminated. */
void B_FN(exit)(void) {
    syscall(SYS_exit, 0);
}

/* The current process splits into two. The child process is
   returned a zero. The parent process is returned the pro-
   cess ID of the child. A negative number returned indicates
   an error. */
B_TYPE B_FN(fork)(void) {
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
    
    if(n < 0) {
        B_FN(putchar)('-');
        n = -n;
    }

    if((a = n / b))
        B_FN(printn)(a, b);
    B_FN(putchar)(n % b + '0');
}

/* The character char is written on the standard output file. */
void B_FN(putchar)(B_TYPE chr) {
    syscall(SYS_write, 1, &chr, sizeof(B_TYPE));
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

/* The current system time is returned in the 1-word vector timev. */
void B_FN(time)(B_TYPE timev) {
    *((B_TYPE*) timev) = syscall(SYS_time, 0);
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
    int child_status;
    return (B_TYPE) syscall(SYS_wait4, -1, &child_status, 0);
}

/* Count bytes are written out of the vector buffer on the
   open file designated by file. The actual number of bytes
   written are returned. A negative number returned indicates
   an error. */
B_TYPE B_FN(nwrite)(B_TYPE file, B_TYPE buffer, B_TYPE count) {
    return (B_TYPE) syscall(SYS_write, file, buffer, count);
}
