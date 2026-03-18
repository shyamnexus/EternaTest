/*
 *  Entropy accumulator implementation
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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "include/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_ENTROPY_C)

#if defined(MBEDTLS_TEST_NULL_ENTROPY)
#warning "**** WARNING!  MBEDTLS_TEST_NULL_ENTROPY defined! "
#warning "**** THIS BUILD HAS NO DEFINED ENTROPY SOURCES "
#warning "**** THIS BUILD IS *NOT* SUITABLE FOR PRODUCTION USE "
#endif

#include "include/entropy.h"
#include "include/entropy_poll.h"
#include "include/platform_util.h"

#include <string.h>

#if defined(MBEDTLS_FS_IO)
#include <stdio.h>
#endif

#if defined(MBEDTLS_ENTROPY_NV_SEED)
#include "include/platform.h"
#endif

#if defined(MBEDTLS_SELF_TEST)
#if defined(MBEDTLS_PLATFORM_C)
#include "include/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif /* MBEDTLS_PLATFORM_C */
#endif /* MBEDTLS_SELF_TEST */

#if defined(MBEDTLS_HAVEGE_C)
#include "mbedtls/havege.h"
#endif

#define ENTROPY_MAX_LOOP    256     /**< Maximum amount to loop before error */

void mbedtls_entropy_init( mbedtls_entropy_context *ctx )
{
    ctx->source_count = 0;
    memset( ctx->source, 0, sizeof( ctx->source ) );

    ctx->accumulator_started = 0;
    mbedtls_sha512_init( &ctx->accumulator );

    /* Reminder: Update ENTROPY_HAVE_STRONG in the test files
     *           when adding more strong entropy sources here. */

#if !defined(MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES)
#if !defined(MBEDTLS_NO_PLATFORM_ENTROPY)
    mbedtls_entropy_add_source( ctx, mbedtls_platform_entropy_poll, NULL,
                                MBEDTLS_ENTROPY_MIN_PLATFORM,
                                MBEDTLS_ENTROPY_SOURCE_STRONG );
#endif
#if defined(MBEDTLS_TIMING_C)
    mbedtls_entropy_add_source( ctx, mbedtls_hardclock_poll, NULL,
                                MBEDTLS_ENTROPY_MIN_HARDCLOCK,
                                MBEDTLS_ENTROPY_SOURCE_WEAK );
#endif
#endif /* MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES */
}

void mbedtls_entropy_free( mbedtls_entropy_context *ctx )
{
    mbedtls_sha512_free( &ctx->accumulator );
    ctx->source_count = 0;
    mbedtls_platform_zeroize( ctx->source, sizeof( ctx->source ) );
    ctx->accumulator_started = 0;
}

int mbedtls_entropy_add_source( mbedtls_entropy_context *ctx,
                        mbedtls_entropy_f_source_ptr f_source, void *p_source,
                        size_t threshold, int strong )
{
    int idx, ret = 0;

    idx = ctx->source_count;
    if( idx >= MBEDTLS_ENTROPY_MAX_SOURCES )
    {
        ret = MBEDTLS_ERR_ENTROPY_MAX_SOURCES;
        goto exit;
    }

    ctx->source[idx].f_source  = f_source;
    ctx->source[idx].p_source  = p_source;
    ctx->source[idx].threshold = threshold;
    ctx->source[idx].strong    = strong;

    ctx->source_count++;

exit:

    return( ret );
}

/*
 * Entropy accumulator update
 */
static int entropy_update( mbedtls_entropy_context *ctx, unsigned char source_id,
                           const unsigned char *data, size_t len )
{
    unsigned char header[2];
    unsigned char tmp[MBEDTLS_ENTROPY_BLOCK_SIZE];
    size_t use_len = len;
    const unsigned char *p = data;
    int ret = 0;

    if( use_len > MBEDTLS_ENTROPY_BLOCK_SIZE )
    {
        if( ( ret = mbedtls_sha512_ret( data, len, tmp, 0 ) ) != 0 )
            goto cleanup;
        p = tmp;
        use_len = MBEDTLS_ENTROPY_BLOCK_SIZE;
    }

    header[0] = source_id;
    header[1] = use_len & 0xFF;

    /*
     * Start the accumulator if this has not already happened. Note that
     * it is sufficient to start the accumulator here only because all calls to
     * gather entropy eventually execute this code.
     */
    if( ctx->accumulator_started == 0 &&
        ( ret = mbedtls_sha512_starts_ret( &ctx->accumulator, 0 ) ) != 0 )
        goto cleanup;
    else
        ctx->accumulator_started = 1;
    if( ( ret = mbedtls_sha512_update_ret( &ctx->accumulator, header, 2 ) ) != 0 )
        goto cleanup;
    ret = mbedtls_sha512_update_ret( &ctx->accumulator, p, use_len );

cleanup:
    mbedtls_platform_zeroize( tmp, sizeof( tmp ) );

    return( ret );
}

/*
 * Run through the different sources to add entropy to our accumulator
 */
static int entropy_gather_internal( mbedtls_entropy_context *ctx )
{
    int ret, i, have_one_strong = 0;
    unsigned char buf[MBEDTLS_ENTROPY_MAX_GATHER];
    size_t olen;

    if( ctx->source_count == 0 )
        return( MBEDTLS_ERR_ENTROPY_NO_SOURCES_DEFINED );

    /*
     * Run through our entropy sources
     */
    for( i = 0; i < ctx->source_count; i++ )
    {
        if( ctx->source[i].strong == MBEDTLS_ENTROPY_SOURCE_STRONG )
            have_one_strong = 1;

        olen = 0;
        if( ( ret = ctx->source[i].f_source( ctx->source[i].p_source,
                        buf, MBEDTLS_ENTROPY_MAX_GATHER, &olen ) ) != 0 )
        {
            goto cleanup;
        }

        /*
         * Add if we actually gathered something
         */
        if( olen > 0 )
        {
            if( ( ret = entropy_update( ctx, (unsigned char) i,
                                        buf, olen ) ) != 0 )
                return( ret );
            ctx->source[i].size += olen;
        }
    }

    if( have_one_strong == 0 )
        ret = MBEDTLS_ERR_ENTROPY_NO_STRONG_SOURCE;

cleanup:
    mbedtls_platform_zeroize( buf, sizeof( buf ) );

    return( ret );
}

int mbedtls_entropy_func( void *data, unsigned char *output, size_t len )
{
    int ret, count = 0, i, done;
    mbedtls_entropy_context *ctx = (mbedtls_entropy_context *) data;
    unsigned char buf[MBEDTLS_ENTROPY_BLOCK_SIZE];

    if( len > MBEDTLS_ENTROPY_BLOCK_SIZE )
        return( MBEDTLS_ERR_ENTROPY_SOURCE_FAILED );

    /*
     * Always gather extra entropy before a call
     */
    do
    {
        if( count++ > ENTROPY_MAX_LOOP )
        {
            ret = MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
            goto exit;
        }

        if( ( ret = entropy_gather_internal( ctx ) ) != 0 )
            goto exit;

        done = 1;
        for( i = 0; i < ctx->source_count; i++ )
            if( ctx->source[i].size < ctx->source[i].threshold )
                done = 0;
    }
    while( ! done );

    memset( buf, 0, MBEDTLS_ENTROPY_BLOCK_SIZE );
    /*
     * Note that at this stage it is assumed that the accumulator was started
     * in a previous call to entropy_update(). If this is not guaranteed, the
     * code below will fail.
     */
    if( ( ret = mbedtls_sha512_finish_ret( &ctx->accumulator, buf ) ) != 0 )
        goto exit;

    /*
     * Reset accumulator and counters and recycle existing entropy
     */
    mbedtls_sha512_free( &ctx->accumulator );
    mbedtls_sha512_init( &ctx->accumulator );
    if( ( ret = mbedtls_sha512_starts_ret( &ctx->accumulator, 0 ) ) != 0 )
        goto exit;
    if( ( ret = mbedtls_sha512_update_ret( &ctx->accumulator, buf,
                                           MBEDTLS_ENTROPY_BLOCK_SIZE ) ) != 0 )
        goto exit;

    /*
     * Perform second SHA-512 on entropy
     */
    if( ( ret = mbedtls_sha512_ret( buf, MBEDTLS_ENTROPY_BLOCK_SIZE,
                                    buf, 0 ) ) != 0 )
        goto exit;

    for( i = 0; i < ctx->source_count; i++ )
        ctx->source[i].size = 0;

    memcpy( output, buf, len );

    ret = 0;

exit:
    mbedtls_platform_zeroize( buf, sizeof( buf ) );

    return( ret );
}

#endif /* MBEDTLS_ENTROPY_C */
