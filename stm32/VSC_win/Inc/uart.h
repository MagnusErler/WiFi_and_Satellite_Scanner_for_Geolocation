

#include "smtc_hal_gpio_pin_names.h"





/*!
 * @file      smtc_hal_dbg_trace.h
 *
 * @brief     Board specific package debug log API definition.
 *
 * Revised BSD License
 * Copyright Semtech Corporation 2020. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SMTC_HAL_DBG_TRACE_H
#define SMTC_HAL_DBG_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>   // C99 types
#include <stdbool.h>  // bool type

#include "uart.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

#define HAL_DBG_TRACE_COLOR_BLACK "\x1B[0;30m"
#define HAL_DBG_TRACE_COLOR_RED "\x1B[0;31m"
#define HAL_DBG_TRACE_COLOR_GREEN "\x1B[0;32m"
#define HAL_DBG_TRACE_COLOR_YELLOW "\x1B[0;33m"
#define HAL_DBG_TRACE_COLOR_BLUE "\x1B[0;34m"
#define HAL_DBG_TRACE_COLOR_MAGENTA "\x1B[0;35m"
#define HAL_DBG_TRACE_COLOR_CYAN "\x1B[0;36m"
#define HAL_DBG_TRACE_COLOR_WHITE "\x1B[0;37m"
#define HAL_DBG_TRACE_COLOR_DEFAULT "\x1B[0m"

#if( UNIT_TEST_DBG )
#define HAL_DBG_TRACE_PRINTF( ... ) printf( __VA_ARGS__ )
#else
#define HAL_DBG_TRACE_PRINTF( ... ) hal_mcu_trace_print( __VA_ARGS__ )
#endif

#define HAL_DBG_TRACE_MSG( msg )                             \
    do                                                       \
    {                                                        \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_DEFAULT ); \
        HAL_DBG_TRACE_PRINTF( msg );                         \
    } while( 0 );

#define HAL_DBG_TRACE_MSG_COLOR( msg, color )                \
    do                                                       \
    {                                                        \
        HAL_DBG_TRACE_PRINTF( color );                       \
        HAL_DBG_TRACE_PRINTF( msg );                         \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_DEFAULT ); \
    } while( 0 );

#define HAL_DBG_TRACE_INFO( ... )                            \
    do                                                       \
    {                                                        \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_GREEN );   \
        HAL_DBG_TRACE_PRINTF( "INFO : " );                   \
        HAL_DBG_TRACE_PRINTF( __VA_ARGS__ );                 \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_DEFAULT ); \
    } while( 0 );

#define HAL_DBG_TRACE_WARNING( ... )                         \
    do                                                       \
    {                                                        \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_YELLOW );  \
        HAL_DBG_TRACE_PRINTF( "WARN : " );                   \
        HAL_DBG_TRACE_PRINTF( __VA_ARGS__ );                 \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_DEFAULT ); \
    } while( 0 );

#define HAL_DBG_TRACE_ERROR( ... )                           \
    do                                                       \
    {                                                        \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_RED );     \
        HAL_DBG_TRACE_PRINTF( "ERROR: " );                   \
        HAL_DBG_TRACE_PRINTF( __VA_ARGS__ );                 \
        HAL_DBG_TRACE_PRINTF( HAL_DBG_TRACE_COLOR_DEFAULT ); \
    } while( 0 );

#define HAL_DBG_TRACE_ARRAY( msg, array, len )                                \
    do                                                                        \
    {                                                                         \
        HAL_DBG_TRACE_PRINTF( "%s - (%lu bytes):\n", msg, ( uint32_t ) len ); \
        for( uint32_t i = 0; i < ( uint32_t ) len; i++ )                      \
        {                                                                     \
            if( ( ( i % 16 ) == 0 ) && ( i > 0 ) )                            \
            {                                                                 \
                HAL_DBG_TRACE_PRINTF( "\n" );                                 \
            }                                                                 \
            HAL_DBG_TRACE_PRINTF( " %02X", array[i] );                        \
        }                                                                     \
        HAL_DBG_TRACE_PRINTF( "\n" );                                         \
    } while( 0 );

#define HAL_DBG_TRACE_PACKARRAY( msg, array, len )       \
    do                                                   \
    {                                                    \
        for( uint32_t i = 0; i < ( uint32_t ) len; i++ ) \
        {                                                \
            HAL_DBG_TRACE_PRINTF( "%02X", array[i] );    \
        }                                                \
    } while( 0 );

#define HAL_PERF_TEST_TRACE_PRINTF( ... )

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

#ifdef __cplusplus
}
#endif

#endif  // SMTC_HAL_DBG_TRACE_H

/* --- EOF ------------------------------------------------------------------ */















/*!
 * @brief Prints debug trace
 *
 * @param variadics arguments
 */
void hal_mcu_trace_print( const char* fmt, ... );

/*!
 * @brief Initializes UART
 *
 * @param id UART id
 * @param uart_tx UART TX pin
 * @param uart_rx UART RX pin
 */
void hal_uart_init( const uint32_t id, const hal_gpio_pin_names_t uart_tx, const hal_gpio_pin_names_t uart_rx );