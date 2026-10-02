#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#if defined( _WIN32)
# include <io.h>
#else
# include <unistd.h>
#endif


#if defined( _WIN32)
# define MULLE_IOVEC_TEST_BINARY  O_BINARY
#else
# define MULLE_IOVEC_TEST_BINARY  0
#endif


int   main( int argc, char *argv[])
{
   char                 *str1;
   char                 *str2;
   char                 buffer1[32];
   char                 buffer2[32];
   int                  fd;
   ssize_t              bytes_written;
   ssize_t              bytes_read;
   struct mulle_iovec   write_iov[2];
   struct mulle_iovec   read_iov[2];

   str1 = "Hello ";
   str2 = "World!\n";

   printf( "Test: Writing two strings with mulle_writev\n");

   // Setup write iovec
   write_iov[0].iov_base = str1;
   write_iov[0].iov_len  = strlen( str1);
   write_iov[1].iov_base = str2;
   write_iov[1].iov_len  = strlen( str2);

   // Create temporary file in the current directory (works on all platforms)
   fd = open( "test_iovec.tmp",
              O_CREAT | O_WRONLY | O_TRUNC | MULLE_IOVEC_TEST_BINARY,
              0644);
   if( fd == -1)
   {
      printf( "Error: Failed to create test file\n");
      return( 1);
   }

   // Write using mulle_writev
   bytes_written = mulle_writev( fd, write_iov, 2);
   close( fd);

   printf( "Bytes written: %d\n", (int) bytes_written);

   printf( "Test: Reading back with mulle_readv\n");

   // Setup read iovec
   memset( buffer1, 0, sizeof( buffer1));
   memset( buffer2, 0, sizeof( buffer2));
   read_iov[0].iov_base = buffer1;
   read_iov[0].iov_len  = strlen( str1);
   read_iov[1].iov_base = buffer2;
   read_iov[1].iov_len  = strlen( str2);

   // Read back using mulle_readv
   fd = open( "test_iovec.tmp", O_RDONLY | MULLE_IOVEC_TEST_BINARY);
   if( fd == -1)
   {
      printf( "Error: Failed to open test file for reading\n");
      return( 1);
   }

   bytes_read = mulle_readv( fd, read_iov, 2);
   close( fd);

   printf( "Bytes read: %d\n", (int) bytes_read);
   printf( "Read content: '%s%s'\n", buffer1, buffer2);

   // Clean up
   remove( "test_iovec.tmp");

   return( 0);
}
