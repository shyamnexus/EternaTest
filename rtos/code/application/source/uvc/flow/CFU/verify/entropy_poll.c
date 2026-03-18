/*
 *  Platform-specific and custom entropy polling functions
 *
 *  Copyright (C) 2006-2016, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if defined(__linux__)
/* Ensure that syscall() is available even when compiling with -std=c99 */
#define _GNU_SOURCE
#endif

#include "include/config.h"
#include <string.h>
#include "include/entropy.h"
#include "include/entropy_poll.h"
#include "include/timing.h"
#include "prjcfg.h"

//local debug level: THIS_DBGLVL
//#define THIS_DBGLVL         6 // 0=FATAL, 1=ERR, 2=WRN, 3=UNIT, 4=FUNC, 5=IND, 6=MSG, 7=VALUE, 8=USER
///////////////////////////////////////////////////////////////////////////////
//#define __MODULE__          UINet
//#define __DBGLVL__          ((THIS_DBGLVL>=PRJ_DBG_LVL)?THIS_DBGLVL:PRJ_DBG_LVL)
//#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include <kwrap/debug.h>
///////////////////////////////////////////////////////////////////////////////

static int getrandom_wrapper( void *buf, size_t buflen, unsigned int flags )
{
    //return( syscall( SYS_getrandom, buf, buflen, flags ) );
	//DBG_ERR("getrandom_wrapper() is not implemented\n");
	return buflen;
}

int mbedtls_platform_entropy_poll( void *data,
                           unsigned char *output, size_t len, size_t *olen )
{
    int ret;
    ((void) data);

    ret = getrandom_wrapper( output, len, 0 );
    if( ret >= 0 )
    {
        *olen = ret;
        return( 0 );
    }
     
	 return( MBEDTLS_ERR_ENTROPY_SOURCE_FAILED );
}

int mbedtls_hardclock_poll( void *data,
                    unsigned char *output, size_t len, size_t *olen )
{
    unsigned long timer = mbedtls_timing_hardclock();
    ((void) data);
    *olen = 0;

    if( len < sizeof(unsigned long) )
        return( 0 );

    memcpy( output, &timer, sizeof(unsigned long) );
    *olen = sizeof(unsigned long);

    return( 0 );
}
