#ifndef _PORTMACRO_DEF_H
#define _PORTMACRO_DEF_H

#define EXCEPTION_LEVEL_NA (0)
#define EXCEPTION_LEVEL_0 (0x0 << 2)
#define EXCEPTION_LEVEL_1 (0x1 << 2)
#define EXCEPTION_LEVEL_2 (0x2 << 2)
#define EXCEPTION_LEVEL_3 (0x3 << 2)

#if defined(__aarch64__)
#define EXPECT_EL EXCEPTION_LEVEL_2
#else
#define EXPECT_EL EXCEPTION_LEVEL_NA
#endif

#endif
