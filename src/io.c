#include <io.h>
#include <stdarg.h>

void eprintf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, IO_RED IO_BOLD "[Error]" IO_RST IO_RED " ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, IO_RST);
}