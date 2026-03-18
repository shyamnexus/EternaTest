#ifndef _PORTMACRO_H
#define _PORTMACRO_H

#include <portmacro-opt.h>
#if defined(__aarch64__)
#include <portmacro-a64.h>
#else
#include <portmacro-a32.h>
#endif

#endif