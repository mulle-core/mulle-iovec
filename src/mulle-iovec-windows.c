/*
 * Windows Implementation Notes:
 *
 * This file provides Windows-specific implementations of mulle_readv and mulle_writev
 * functions that emulate the POSIX readv(2) and writev(2) system calls.
 *
 * Implementation Details:
 * - Uses _get_osfhandle() to convert POSIX file descriptors to Windows HANDLEs
 * - Loops through each iovec buffer calling ReadFile/WriteFile for each one
 * - ReadFile/WriteFile take the byte count as a DWORD, so a single iovec longer
 *   than MAXDWORD is split into multiple transfers (the POSIX kernel does the
 *   same with MAX_RW_COUNT). The split limit is configurable for testing.
 * - For reads: stops early if a buffer isn't completely filled (matching readv behavior)
 * - For writes: stops at the first short write, so later buffers are never
 *   written as if the earlier one had completed (matching writev behavior)
 * - Properly handles error conditions and sets errno appropriately
 * - Returns total number of bytes read/written across all buffers
 *
 * Why not ReadFileScatter/WriteFileGather:
 * While Windows provides ReadFileScatter and WriteFileGather functions that are closer
 * to the native readv/writev behavior, they require:
 * 1. Buffers to be page-aligned in memory
 * 2. File handles opened with FILE_FLAG_OVERLAPPED
 * 3. Buffer sizes to be multiples of the system page size
 *
 * Since this is a general-purpose portability layer, we cannot guarantee that
 * callers will provide page-aligned buffers or use overlapped I/O. The loop-based
 * approach using ReadFile/WriteFile ensures compatibility with any buffer alignment
 * and file handle type, trading some performance for universal correctness.
 */

#include "include-private.h"
#include "mulle-iovec-windows.h"
#include "mulle-iovec.h"
#include "mulle-iovec-private.h"

#ifdef _WIN32

#include <windows.h>
#include <io.h>
#include <errno.h>


static DWORD   mulle_iovec_windows_max_transfer = MAXDWORD;


void   mulle_iovec_windows_set_max_transfer( size_t value)
{
   if( value == 0 || value > (size_t) MAXDWORD)
      value = (size_t) MAXDWORD;

   mulle_iovec_windows_max_transfer = (DWORD) value;
}


static int
   mulle_iovec_errno_map( void)
{
   switch( GetLastError())
   {
   case ERROR_INVALID_HANDLE : return( EBADF);
   case ERROR_ACCESS_DENIED  :
   case ERROR_INVALID_ACCESS : return( EACCES);
   case ERROR_BROKEN_PIPE     :
   case ERROR_NO_DATA         : return( EPIPE);
   case ERROR_DISK_FULL       : return( ENOSPC);
   default                    : return( EIO);
   }
}


ssize_t   mulle_readv_windows( int fd, struct mulle_iovec *iov, int iovcnt)
{
   HANDLE  handle;
   ssize_t total_read = 0;
   int     i;
   size_t  done;
   size_t  chunk;
   DWORD   bytes_read_now;

   if( mulle_iovec_validate_arguments( iov, iovcnt))
      return( -1);

   handle = (HANDLE) _get_osfhandle( fd);
   if( handle == INVALID_HANDLE_VALUE)
   {
      errno = EBADF;
      return( -1);
   }

   for( i = 0; i < iovcnt; i++)
   {
      done = 0;
      while( done < iov[i].iov_len)
      {
         chunk = iov[i].iov_len - done;
         if( chunk > (size_t) mulle_iovec_windows_max_transfer)
            chunk = (size_t) mulle_iovec_windows_max_transfer;

         if( ! ReadFile( handle, (char *) iov[i].iov_base + done,
                         (DWORD) chunk, &bytes_read_now, NULL))
         {
            errno = mulle_iovec_errno_map();
            if( total_read)
               return( total_read);
            return( -1);
         }

         total_read += bytes_read_now;
         done       += bytes_read_now;
         if( bytes_read_now < chunk)
            return( total_read);
      }
   }
   return( total_read);
}


ssize_t   mulle_writev_windows( int fd, struct mulle_iovec *iov, int iovcnt)
{
   HANDLE  handle;
   ssize_t total_written = 0;
   int     i;
   size_t  done;
   size_t  chunk;
   DWORD   bytes_written_now;

   if( mulle_iovec_validate_arguments( iov, iovcnt))
      return( -1);

   handle = (HANDLE) _get_osfhandle( fd);
   if( handle == INVALID_HANDLE_VALUE)
   {
      errno = EBADF;
      return( -1);
   }

   for( i = 0; i < iovcnt; i++)
   {
      done = 0;
      while( done < iov[i].iov_len)
      {
         chunk = iov[i].iov_len - done;
         if( chunk > (size_t) mulle_iovec_windows_max_transfer)
            chunk = (size_t) mulle_iovec_windows_max_transfer;

         if( ! WriteFile( handle, (char *) iov[i].iov_base + done,
                          (DWORD) chunk, &bytes_written_now, NULL))
         {
            errno = mulle_iovec_errno_map();
            if( total_written)
               return( total_written);
            return( -1);
         }

         total_written += bytes_written_now;
         done          += bytes_written_now;
         if( bytes_written_now < chunk)
            return( total_written);
      }
   }
   return( total_written);
}

#endif
