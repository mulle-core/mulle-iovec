#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>


int   main( int argc, char *argv[])
{
   struct mulle_iovec   w[ 3];
   struct mulle_iovec   r[ 2];
   char                 buffer[ 32];
   int                  fds[ 2];
   ssize_t              n;

   if( pipe( fds) != 0)
   {
      printf( "Error: Failed to create pipe\n");
      return( 1);
   }

   w[0].iov_base = (void *) "Hello, ";
   w[0].iov_len  = 7;
   w[1].iov_base = (void *) "World";
   w[1].iov_len  = 5;
   w[2].iov_base = (void *) "!";
   w[2].iov_len  = 1;

   n = mulle_writev( fds[1], w, 3);
   printf( "pipe write: %d\n", (int) n);

   memset( buffer, 0, sizeof( buffer));
   r[0].iov_base = buffer;
   r[0].iov_len  = 8;
   r[1].iov_base = &buffer[ 8];
   r[1].iov_len  = 8;

   n = mulle_readv( fds[0], r, 2);
   printf( "pipe read: %d\n", (int) n);
   printf( "pipe content: '%s'\n", buffer);

   close( fds[0]);
   close( fds[1]);

   if( pipe( fds) != 0)
   {
      printf( "Error: Failed to create pipe\n");
      return( 1);
   }

   w[0].iov_base = (void *) "abc";
   w[0].iov_len  = 3;

   n = mulle_writev( fds[1], w, 1);
   printf( "short write: %d\n", (int) n);

   memset( buffer, 0, sizeof( buffer));
   n = mulle_readv( fds[0], r, 2);
   printf( "short read: %d\n", (int) n);
   printf( "short content: '%s'\n", buffer);

   close( fds[0]);
   close( fds[1]);

   return( 0);
}
