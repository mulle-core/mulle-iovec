#include <mulle-iovec/mulle-iovec.h>

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


#define TEST_COUNT   100
#define TEST_LENGTH  100


int   main( int argc, char *argv[])
{
   struct mulle_iovec   iov[ TEST_COUNT];
   char                 data[ TEST_COUNT][ TEST_LENGTH];
   char                 buffer[ TEST_COUNT * TEST_LENGTH];
   int                  fd;
   int                  i;
   int                  j;
   int                  mismatches;
   ssize_t              n;
   ssize_t              total;

   for( i = 0; i < TEST_COUNT; i++)
   {
      for( j = 0; j < TEST_LENGTH; j++)
         data[i][j] = (char) ('A' + ((i + j) % 26));

      iov[i].iov_base = data[i];
      iov[i].iov_len  = TEST_LENGTH;
   }

   fd = open( "writev_all.tmp",
              O_CREAT | O_WRONLY | O_TRUNC | TEST_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }

   n = mulle_writev_all( fd, iov, TEST_COUNT);
   close( fd);
   printf( "writev_all: %d\n", (int) n);

   fd = open( "writev_all.tmp", O_RDONLY | TEST_BINARY);
   memset( buffer, 0, sizeof( buffer));
   total = 0;
   while( total < (ssize_t) sizeof( buffer))
   {
      n = read( fd, &buffer[ total], sizeof( buffer) - total);
      if( n <= 0)
         break;
      total += n;
   }
   close( fd);
   remove( "writev_all.tmp");

   mismatches = 0;
   for( i = 0; i < TEST_COUNT; i++)
      for( j = 0; j < TEST_LENGTH; j++)
         if( buffer[ i * TEST_LENGTH + j] != data[i][j])
            mismatches++;

   printf( "read: %d\n", (int) total);
   printf( "verify: %s\n", mismatches ? "FAIL" : "ok");

   fd = open( "writev_all.tmp", O_CREAT | O_RDWR | O_TRUNC | TEST_BINARY, 0644);
   n  = mulle_writev_all( fd, iov, 0);
   close( fd);
   remove( "writev_all.tmp");
   printf( "writev_all 0 iovecs: %d\n", (int) n);

   return( 0);
}
