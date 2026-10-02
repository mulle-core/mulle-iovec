#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>


int   main( int argc, char *argv[])
{
   uint32_t   version;
   int        flag;

   version = mulle_iovec_get_version();
   flag    = (mulle_iovec_get_version_major() == (version >> 20)) &&
             (mulle_iovec_get_version_minor() == ((version >> 8) & 0xFFF)) &&
             (mulle_iovec_get_version_patch() == (version & 0xFF));

   printf( "get_version consistent: %s\n", flag ? "YES" : "NO");

   return( 0);
}
