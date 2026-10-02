#include <mulle-iovec/mulle-iovec.h>
#include <mulle-iovec/mulle-iovec-private.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>


#define TEST_TOTAL   100000
#define TEST_LIMIT   4096


int   main( int argc, char *argv[])
{
   struct mulle_iovec   iov[ 3];
   char                 *data;
   char                 *back;
   int                  fd;
   int                  i;
   int                  mismatches;
   ssize_t              n;

   data = malloc( TEST_TOTAL);
   back = malloc( TEST_TOTAL);
   if( ! data || ! back)
      return( 1);

   for( i = 0; i < TEST_TOTAL; i++)
      data[i] = (char) ('A' + (i % 26));

   mulle_iovec_windows_set_max_transfer( TEST_LIMIT);

   fd = open( "windows_chunk.tmp",
              O_CREAT | O_RDWR | O_TRUNC | O_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }

   iov[0].iov_base = data;
   iov[0].iov_len  = TEST_TOTAL;
   n = mulle_writev( fd, iov, 1);
   printf( "writev: %d\n", (int) n);

   _lseek( fd, 0, SEEK_SET);
   n = mulle_writev_all( fd, iov, 1);
   printf( "writev_all: %d\n", (int) n);

   _lseek( fd, 0, SEEK_SET);
   memset( back, 0, TEST_TOTAL);
   iov[0].iov_base = back;
   iov[0].iov_len  = TEST_TOTAL;
   n = mulle_readv( fd, iov, 1);
   printf( "readv: %d\n", (int) n);

   mismatches = 0;
   for( i = 0; i < TEST_TOTAL; i++)
      if( back[i] != data[i])
         mismatches++;
   printf( "verify: %s\n", mismatches ? "FAIL" : "ok");

   close( fd);
   remove( "windows_chunk.tmp");

   iov[0].iov_base = data;
   iov[0].iov_len  = 5000;
   iov[1].iov_base = data + 5000;
   iov[1].iov_len  = 0;
   iov[2].iov_base = data + 5000;
   iov[2].iov_len  = 7000;

   fd = open( "windows_chunk.tmp",
              O_CREAT | O_RDWR | O_TRUNC | O_BINARY,
              0644);
   n = mulle_writev( fd, iov, 3);
   printf( "multi writev: %d\n", (int) n);

   _lseek( fd, 0, SEEK_SET);
   memset( back, 0, TEST_TOTAL);
   iov[0].iov_base = back;
   iov[0].iov_len  = 5000;
   iov[1].iov_base = back + 5000;
   iov[1].iov_len  = 0;
   iov[2].iov_base = back + 5000;
   iov[2].iov_len  = 7000;
   n = mulle_readv( fd, iov, 3);
   printf( "multi readv: %d\n", (int) n);

   mismatches = 0;
   for( i = 0; i < 12000; i++)
      if( back[i] != data[i])
         mismatches++;
   printf( "multi verify: %s\n", mismatches ? "FAIL" : "ok");

   close( fd);
   remove( "windows_chunk.tmp");

   free( back);
   free( data);

   return( 0);
}
