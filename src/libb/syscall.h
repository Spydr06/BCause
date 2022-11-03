#ifndef __LIBB_SYSCALL_H
#define __LIBB_SYSCALL_H

/* VA aliases */
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
#define va_list __builtin_va_list

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

static inline SYSCALL_TYPE __syscall4(SYSCALL_TYPE n, SYSCALL_TYPE a1, SYSCALL_TYPE a2, SYSCALL_TYPE a3, SYSCALL_TYPE a4)
{
	unsigned SYSCALL_TYPE ret;
	register SYSCALL_TYPE r10 __asm__("r10") = a4;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10): "rcx", "r11", "memory");
	return ret;
}

static inline SYSCALL_TYPE __syscall5(SYSCALL_TYPE n, SYSCALL_TYPE a1, SYSCALL_TYPE a2, SYSCALL_TYPE a3, SYSCALL_TYPE a4, SYSCALL_TYPE a5)
{
	unsigned SYSCALL_TYPE ret;
	register SYSCALL_TYPE r10 __asm__("r10") = a4;
	register SYSCALL_TYPE r8 __asm__("r8") = a5;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
	return ret;
}

static inline SYSCALL_TYPE __syscall6(SYSCALL_TYPE n, SYSCALL_TYPE a1, SYSCALL_TYPE a2, SYSCALL_TYPE a3, SYSCALL_TYPE a4, SYSCALL_TYPE a5, SYSCALL_TYPE a6)
{
	unsigned SYSCALL_TYPE ret;
	register SYSCALL_TYPE r10 __asm__("r10") = a4;
	register SYSCALL_TYPE r8 __asm__("r8") = a5;
	register SYSCALL_TYPE r9 __asm__("r9") = a6;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
	return ret;
}

#define __scc(X) ((SYSCALL_TYPE) (X))

#define __syscall1(n,a) __syscall1(n,__scc(a))
#define __syscall2(n,a,b) __syscall2(n,__scc(a),__scc(b))
#define __syscall3(n,a,b,c) __syscall3(n,__scc(a),__scc(b),__scc(c))
#define __syscall4(n,a,b,c,d) __syscall4(n,__scc(a),__scc(b),__scc(c),__scc(d))
#define __syscall5(n,a,b,c,d,e) __syscall5(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e))
#define __syscall6(n,a,b,c,d,e,f) __syscall6(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f))
#define __syscall7(n,a,b,c,d,e,f,g) __syscall7(n,__scc(a),__scc(b),__scc(c),__scc(d),__scc(e),__scc(f),__scc(g))

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) __SYSCALL_NARGS_X(__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b) a##b
#define __SYSCALL_CONCAT(a,b) __SYSCALL_CONCAT_X(a,b)
#define __SYSCALL_DISP(b,...) __SYSCALL_CONCAT(b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __syscall(...) __SYSCALL_DISP(__syscall,__VA_ARGS__)
#define syscall(...) __syscall_ret(__syscall(__VA_ARGS__))

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
#define SYS_getuid 102
#define SYS_setuid 105
#define SYS_time 201

#endif /* __LIBB_SYSCALL */
