//
//  bench.h
//  mulle-iovec benchmarks
//
//  Shared helper for the timing benchmark in test/bench/. There is no
//  golden .stdout: timings are noisy, the test harness only smoke-runs it.
//
#ifndef bench_h__
#define bench_h__

#ifndef _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200809L
#endif

#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>
#include <stdint.h>
#include <time.h>


#if defined( _WIN32)

# include <windows.h>

static uint64_t  bench_now( void)
{
   LARGE_INTEGER   counter;
   LARGE_INTEGER   frequency;

   QueryPerformanceCounter( &counter);
   QueryPerformanceFrequency( &frequency);
   return( (uint64_t) ((counter.QuadPart * 1000000000LL) / frequency.QuadPart));
}

#else

static uint64_t  bench_now( void)
{
   struct timespec   ts;

   clock_gettime( CLOCK_MONOTONIC, &ts);
   return( (uint64_t) ts.tv_sec * 1000000000ULL + (uint64_t) ts.tv_nsec);
}

#endif


static inline void  bench_warn_if_debug( void)
{
#ifndef NDEBUG
   fprintf( stderr, "WARNING: asserts enabled (NDEBUG not defined) - timings are not meaningful\n");
#endif
}

#endif
