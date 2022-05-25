#pragma once

#include <stdio.h>

#define IO_RST "\033[0m"
#define IO_BOLD "\033[1m"
#define IO_RED "\033[31m"

void eprintf(const char* fmt, ...);