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
   struct mulle_iovec   iov[ 3];
   char                 buffer[ 32];
   int                  fd;
   ssize_t              n;
   ssize_t              total;

   iov[0].iov_base = (void *) "Hello ";
   iov[0].iov_len  = 6;
   iov[1].iov_base = (void *) "";
   iov[1].iov_len  = 0;
   iov[2].iov_base = (void *) "World!\n";
   iov[2].iov_len  = 7;

   fd = open( "writev.tmp",
              O_CREAT | O_WRONLY | O_TRUNC | TEST_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }

   n = mulle_writev( fd, iov, 3);
   close( fd);
   printf( "write 3 iovecs: %d\n", (int) n);

   fd = open( "writev.tmp", O_RDONLY | TEST_BINARY);
   memset( buffer, 0, sizeof( buffer));
   total = 0;
   while( total < (ssize_t) sizeof( buffer) - 1)
   {
      n = read( fd, &buffer[ total], 1);
      if( n <= 0)
         break;
      total += n;
   }
   close( fd);
   printf( "content: '%s'\n", buffer);
   remove( "writev.tmp");

   fd = open( "writev.tmp", O_CREAT | O_RDWR | O_TRUNC | TEST_BINARY, 0644);
   n  = mulle_writev( fd, iov, 0);
   printf( "write 0 iovecs: %d\n", (int) n);
   close( fd);
   remove( "writev.tmp");

   errno = 0;
   n     = mulle_writev( -1, iov, 1);
   printf( "write bad fd: %s\n", n == -1 ? errno_name( errno) : "unexpected");

   return( 0);
}
