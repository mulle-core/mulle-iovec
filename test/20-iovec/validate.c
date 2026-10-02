#include <mulle-iovec/mulle-iovec-private.h>

#include <errno.h>
#include <stdio.h>


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
   size_t               max_ssize;
   size_t               len;
   int                  r;

   iov[0].iov_base = (void *) "a";
   iov[0].iov_len  = 1;
   iov[1].iov_base = (void *) "bc";
   iov[1].iov_len  = 2;

   errno = 0;
   len   = mulle_iovec_get_length( iov, 2);
   printf( "get_length(2) = %d\n", (int) len);

   errno = 0;
   len   = mulle_iovec_get_length( iov, 0);
   printf( "get_length(0) = %d\n", (int) len);

   errno = 0;
   len   = mulle_iovec_get_length( iov, -1);
   printf( "get_length(-1): %s\n", len == (size_t) -1 ? errno_name( errno) : "unexpected");

   errno = 0;
   len   = mulle_iovec_get_length( iov, 1025);
   printf( "get_length(IOV_MAX+1): %s\n", len == (size_t) -1 ? errno_name( errno) : "unexpected");

   errno = 0;
   len   = mulle_iovec_get_length( NULL, 1);
   printf( "get_length(NULL,1): %s\n", len == (size_t) -1 ? errno_name( errno) : "unexpected");

   max_ssize      = (size_t) ((((size_t) 1) << (sizeof( ssize_t) * 8 - 1)) - 1);
   iov[0].iov_len = max_ssize;
   iov[1].iov_len = 2;

   errno = 0;
   len   = mulle_iovec_get_length( iov, 2);
   printf( "get_length(overflow): %s\n", len == (size_t) -1 ? errno_name( errno) : "unexpected");

   iov[0].iov_len = 1;
   iov[1].iov_len = 2;

   errno = 0;
   r     = mulle_iovec_validate_arguments( iov, 2);
   printf( "validate(2) = %d\n", r);

   errno = 0;
   r     = mulle_iovec_validate_arguments( iov, -1);
   printf( "validate(-1): %s\n", r == -1 ? errno_name( errno) : "unexpected");

   errno = 0;
   r     = mulle_iovec_validate_arguments( iov, 1025);
   printf( "validate(IOV_MAX+1): %s\n", r == -1 ? errno_name( errno) : "unexpected");

   errno = 0;
   r     = mulle_iovec_validate_arguments( NULL, 1);
   printf( "validate(NULL,1): %s\n", r == -1 ? errno_name( errno) : "unexpected");

   errno = 0;
   r     = mulle_iovec_validate_arguments( iov, 0);
   printf( "validate(0) = %d\n", r);

   return( 0);
}
