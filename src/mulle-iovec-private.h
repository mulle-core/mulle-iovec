#ifndef mulle__iovec_private_h__
#define mulle__iovec_private_h__

/*
 * Internal helpers, not part of the public API. They are exposed here so
 * that the test suite can verify argument-validation behavior directly.
 */

#include "mulle-iovec.h"

MULLE__IOVEC_GLOBAL
int      mulle_iovec_validate_arguments( struct mulle_iovec *iov, int iovcnt);

MULLE__IOVEC_GLOBAL
size_t   mulle_iovec_get_length( struct mulle_iovec *iov, int iovcnt);

/*
 * Testing hook for the Windows backend: limits how many bytes a single
 * ReadFile/WriteFile transfer may cover. 0 or > MAXDWORD resets it to
 * MAXDWORD. Only available on Windows.
 */
#ifdef _WIN32
MULLE__IOVEC_GLOBAL
void     mulle_iovec_windows_set_max_transfer( size_t value);
#endif

#endif
