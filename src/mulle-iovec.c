#include "include-private.h"

#include "mulle-iovec.h"


int   __MULLE__IOVEC_ranlib__;


uint32_t   mulle_iovec_get_version( void)
{
   return( MULLE__IOVEC_VERSION);
}


#ifdef MULLE_IOVEC_EMULATED
# include <errno.h>
# include <unistd.h>

/*
 * Emulated Implementation Notes:
 *
 * This provides fallback implementations of mulle_readv and mulle_writev
 * for systems that don't have native readv/writev support.
 *
 * Implementation:
 * - Loops through each iovec buffer calling read()/write() for each one
 * - For reads: stops early if a buffer isn't completely filled (matching readv behavior)
 * - Returns total number of bytes read/written across all buffers
 * - Maintains same error semantics as native readv/writev
 */

ssize_t
   mulle_readv( int fd, struct mulle_iovec *iov, int iovcnt)
{
   ssize_t total_read = 0;
   ssize_t bytes_read_now;
   int     i;

   for( i = 0; i < iovcnt; i++)
   {
      bytes_read_now = read( fd, iov[i].iov_base, iov[i].iov_len);
      if( bytes_read_now == -1)
         return( -1);
      
      total_read += bytes_read_now;
      if( bytes_read_now < (ssize_t) iov[i].iov_len)
         break;
   }
   return( total_read);
}


ssize_t
   mulle_writev( int fd, struct mulle_iovec *iov, int iovcnt)
{
   ssize_t total_written = 0;
   ssize_t bytes_written_now;
   int     i;

   for( i = 0; i < iovcnt; i++)
   {
      bytes_written_now = write( fd, iov[i].iov_base, iov[i].iov_len);
      if( bytes_written_now == -1)
         return( -1);
      
      total_written += bytes_written_now;
   }
   return( total_written);
}
#endif
