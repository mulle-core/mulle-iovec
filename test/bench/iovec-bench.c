//
//  iovec-bench.c
//  mulle-iovec
//
//  Wall-clock benchmark: compares batch scatter/gather I/O
//  (mulle_writev/mulle_readv, one call for N buffers) against the
//  per-buffer baseline (N write()/read() calls).
//
//  There is deliberately no .stdout file, so the test harness runs it
//  (smoke test) but does not compare the output. Run it directly or via
//    mulle-sde test --release run test/bench/iovec-bench.c
//  For meaningful numbers use a release build; N defaults small anyway.
//
//  Results are noisy; treat them as ballpark figures.
//
#include "bench.h"

#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#if defined( _WIN32)
# include <io.h>
# define TEST_BINARY   O_BINARY
# define TEST_SEEK     _lseek
#else
# include <unistd.h>
# define TEST_BINARY   0
# define TEST_SEEK     lseek
#endif


#define DEFAULT_N      512
#define MAX_N          4096
#define CHUNK          64
#define DEFAULT_ITERS  1000
#define MAX_ITERS      100000


static size_t   g_n     = DEFAULT_N;
static size_t   g_iters = DEFAULT_ITERS;


static void   report( char *label, uint64_t elapsed, size_t bytes)
{
   double   ns_per_batch;
   double   mb_per_second;

   ns_per_batch  = (double) elapsed / (double) g_iters;
   mb_per_second = elapsed
                     ? (double) bytes * (double) g_iters * 1000.0 / (double) elapsed
                     : 0.0;

   fprintf( stderr, "%-16s %10.1f ns/batch  %8.2f MB/s\n",
            label,
            ns_per_batch,
            mb_per_second);
}


int   main( int argc, char *argv[])
{
   struct mulle_iovec   *iov;
   char                 **buffers;
   uint64_t             t0;
   uint64_t             t1;
   size_t               bytes;
   size_t               iter;
   size_t               i;
   int                  fd;
   ssize_t              n;

   if( argc > 1)
   {
      g_n = (size_t) strtoul( argv[ 1], NULL, 10);
      if( ! g_n || g_n > MAX_N)
         g_n = DEFAULT_N;
   }
   if( argc > 2)
   {
      g_iters = (size_t) strtoul( argv[ 2], NULL, 10);
      if( ! g_iters || g_iters > MAX_ITERS)
         g_iters = DEFAULT_ITERS;
   }

   bytes = g_n * CHUNK;

   iov     = malloc( g_n * sizeof( struct mulle_iovec));
   buffers = malloc( g_n * sizeof( char *));
   if( ! iov || ! buffers)
   {
      fprintf( stderr, "out of memory\n");
      return( 1);
   }

   for( i = 0; i < g_n; i++)
   {
      buffers[i] = malloc( CHUNK);
      memset( buffers[i], (int) ('A' + (i % 26)), CHUNK);

      iov[i].iov_base = buffers[i];
      iov[i].iov_len  = CHUNK;
   }

   fd = open( "iovec-bench.tmp",
              O_CREAT | O_RDWR | O_TRUNC | TEST_BINARY,
              0644);
   if( fd == -1)
   {
      fprintf( stderr, "cannot create benchmark file\n");
      return( 1);
   }

   bench_warn_if_debug();
   fprintf( stderr, "mulle-iovec wall-clock benchmark (N=%zu chunk=%d iters=%zu)\n",
            g_n, CHUNK, g_iters);

   t0 = bench_now();
   for( iter = 0; iter < g_iters; iter++)
   {
      TEST_SEEK( fd, 0, SEEK_SET);
      n = mulle_writev( fd, iov, (int) g_n);
   }
   t1 = bench_now();
   report( "mulle_writev", t1 - t0, bytes);

   t0 = bench_now();
   for( iter = 0; iter < g_iters; iter++)
   {
      TEST_SEEK( fd, 0, SEEK_SET);
      for( i = 0; i < g_n; i++)
         n = write( fd, iov[i].iov_base, iov[i].iov_len);
   }
   t1 = bench_now();
   report( "write baseline", t1 - t0, bytes);

   t0 = bench_now();
   for( iter = 0; iter < g_iters; iter++)
   {
      TEST_SEEK( fd, 0, SEEK_SET);
      n = mulle_writev_all( fd, iov, (int) g_n);
   }
   t1 = bench_now();
   report( "mulle_writev_all", t1 - t0, bytes);

   t0 = bench_now();
   for( iter = 0; iter < g_iters; iter++)
   {
      TEST_SEEK( fd, 0, SEEK_SET);
      n = mulle_readv( fd, iov, (int) g_n);
   }
   t1 = bench_now();
   report( "mulle_readv", t1 - t0, bytes);

   t0 = bench_now();
   for( iter = 0; iter < g_iters; iter++)
   {
      TEST_SEEK( fd, 0, SEEK_SET);
      for( i = 0; i < g_n; i++)
         n = read( fd, iov[i].iov_base, iov[i].iov_len);
   }
   t1 = bench_now();
   report( "read baseline", t1 - t0, bytes);

   close( fd);
   remove( "iovec-bench.tmp");

   (void) n;

   for( i = 0; i < g_n; i++)
      free( buffers[i]);
   free( buffers);
   free( iov);

   return( 0);
}
