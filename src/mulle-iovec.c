#include "include-private.h"

#include "mulle-iovec.h"
#include "mulle-iovec-private.h"

#include <errno.h>
#include <limits.h>


#ifndef IOV_MAX
# define IOV_MAX  1024
#endif

#ifndef SSIZE_MAX
# define SSIZE_MAX  ((ssize_t) ((((size_t) 1) << (sizeof( ssize_t) * CHAR_BIT - 1)) - 1))
#endif


int   __MULLE__IOVEC_ranlib__;


uint32_t   mulle_iovec_get_version( void)
{
   return( MULLE__IOVEC_VERSION);
}


/*
 * Shared argument validation used by the emulated and Windows backends.
 * Mirrors the native readv/writev contract: negative or over-limit iovcnt,
 * a null iov with a positive count, or an aggregate length that would
 * overflow ssize_t are all EINVAL.
 */

int
   mulle_iovec_validate_arguments( struct mulle_iovec *iov, int iovcnt)
{
   size_t   total;
   int      i;

   if( iovcnt < 0 || iovcnt > IOV_MAX)
   {
      errno = EINVAL;
      return( -1);
   }
   if( iovcnt > 0 && ! iov)
   {
      errno = EINVAL;
      return( -1);
   }

   total = 0;
   for( i = 0; i < iovcnt; i++)
   {
      if( iov[i].iov_len > (size_t) SSIZE_MAX - total)
      {
         errno = EINVAL;
         return( -1);
      }
      total += iov[i].iov_len;
   }
   return( 0);
}


size_t
   mulle_iovec_get_length( struct mulle_iovec *iov, int iovcnt)
{
   size_t   total;
   int      i;

   if( iovcnt < 0 || iovcnt > IOV_MAX)
   {
      errno = EINVAL;
      return( (size_t) -1);
   }
   if( iovcnt > 0 && ! iov)
   {
      errno = EINVAL;
      return( (size_t) -1);
   }

   total = 0;
   for( i = 0; i < iovcnt; i++)
   {
      if( iov[i].iov_len > (size_t) SSIZE_MAX - total)
      {
         errno = EINVAL;
         return( (size_t) -1);
      }
      total += iov[i].iov_len;
   }
   return( total);
}


#ifdef MULLE_IOVEC_EMULATED
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
 * - For writes: stops at the first short write, so later buffers are never
 *   written as if the earlier one had completed (matching writev behavior)
 * - Returns total number of bytes read/written across all buffers
 * - Validates arguments and the aggregate length like native readv/writev
 * - Retries EINTR internally, never losing already-transferred byte counts
 */

ssize_t
   mulle_readv( int fd, struct mulle_iovec *iov, int iovcnt)
{
   ssize_t total_read = 0;
   ssize_t bytes_read_now;
   int     i;

   if( mulle_iovec_validate_arguments( iov, iovcnt))
      return( -1);

   for( i = 0; i < iovcnt; i++)
   {
      do
         bytes_read_now = read( fd, iov[i].iov_base, iov[i].iov_len);
      while( bytes_read_now < 0 && errno == EINTR);

      if( bytes_read_now < 0)
      {
         if( total_read)
            return( total_read);
         return( -1);
      }

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

   if( mulle_iovec_validate_arguments( iov, iovcnt))
      return( -1);

   for( i = 0; i < iovcnt; i++)
   {
      do
         bytes_written_now = write( fd, iov[i].iov_base, iov[i].iov_len);
      while( bytes_written_now < 0 && errno == EINTR);

      if( bytes_written_now < 0)
      {
         if( total_written)
            return( total_written);
         return( -1);
      }

      total_written += bytes_written_now;
      if( bytes_written_now < (ssize_t) iov[i].iov_len)
         break;
   }
   return( total_written);
}
#endif
