#include <mulle-iovec/mulle-iovec.h>

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


static char   *errno_name( int no)
{
   switch( no)
   {
   case EINVAL : return( "EINVAL");
   case EBADF  : return( "EBADF");
   case EIO    : return( "EIO");
   default     : return( "other");
   }
}


int   main( int argc, char *argv[])
{
   struct mulle_iovec   iov[ 2];
   char                 buffer1[ 16];
   char                 buffer2[ 16];
   int                  fd;
   ssize_t              n;

   fd = open( "readv.tmp",
              O_CREAT | O_WRONLY | O_TRUNC | TEST_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }
   write( fd, "Hello World!", 12);
   close( fd);

   memset( buffer1, 0, sizeof( buffer1));
   memset( buffer2, 0, sizeof( buffer2));

   iov[0].iov_base = buffer1;
   iov[0].iov_len  = 8;
   iov[1].iov_base = buffer2;
   iov[1].iov_len  = 8;

   fd = open( "readv.tmp", O_RDONLY | TEST_BINARY);
   if( fd == -1)
   {
      printf( "Error: Failed to open test file\n");
      return( 1);
   }

   n = mulle_readv( fd, iov, 2);
   printf( "read 2 iovecs: %d\n", (int) n);
   printf( "buffer1: '%s'\n", buffer1);
   printf( "buffer2: '%s'\n", buffer2);

   n = mulle_readv( fd, iov, 2);
   printf( "read at eof: %d\n", (int) n);
   close( fd);

   fd = open( "readv.tmp", O_RDONLY | TEST_BINARY);
   n  = mulle_readv( fd, iov, 0);
   printf( "read 0 iovecs: %d\n", (int) n);
   close( fd);
   remove( "readv.tmp");

   errno = 0;
   n     = mulle_readv( -1, iov, 1);
   printf( "read bad fd: %s\n", n == -1 ? errno_name( errno) : "unexpected");

   return( 0);
}
