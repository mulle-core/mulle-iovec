#include <mulle-iovec/mulle-iovec.h>
#include <mulle-iovec/mulle-iovec-private.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#if defined( _WIN32)
# include <io.h>
# define TEST_BINARY   O_BINARY
#else
# include <unistd.h>
# define TEST_BINARY   0
#endif


#define TEST_COUNT   1024


int   main( int argc, char *argv[])
{
   struct mulle_iovec   iov[ TEST_COUNT];
   char                 buffer[ TEST_COUNT];
   int                  fd;
   int                  i;
   int                  mismatches;
   int                  r;
   ssize_t              n;

   for( i = 0; i < TEST_COUNT; i++)
   {
      buffer[i] = (char) ('A' + (i % 26));
      iov[i].iov_base = &buffer[ i];
      iov[i].iov_len  = 1;
   }

   fd = open( "boundary.tmp",
              O_CREAT | O_WRONLY | O_TRUNC | TEST_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }

   n = mulle_writev( fd, iov, TEST_COUNT);
   close( fd);
   printf( "writev IOV_MAX: %d\n", (int) n);

   memset( buffer, 0, sizeof( buffer));
   fd = open( "boundary.tmp", O_RDONLY | TEST_BINARY);
   n  = mulle_readv( fd, iov, TEST_COUNT);
   close( fd);
   printf( "readv IOV_MAX: %d\n", (int) n);

   mismatches = 0;
   for( i = 0; i < TEST_COUNT; i++)
      if( buffer[i] != (char) ('A' + (i % 26)))
         mismatches++;

   printf( "verify: %s\n", mismatches ? "FAIL" : "ok");
   remove( "boundary.tmp");

   errno = 0;
   r     = mulle_iovec_validate_arguments( iov, TEST_COUNT);
   printf( "validate(IOV_MAX) = %d\n", r);

   iov[0].iov_len = 0;
   iov[1].iov_len = 0;
   iov[2].iov_len = 5;

   errno = 0;
   printf( "get_length(zeroes) = %d\n", (int) mulle_iovec_get_length( iov, 3));

   return( 0);
}
