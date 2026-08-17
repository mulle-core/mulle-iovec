#ifndef mulle__iovec_private_h__
#define mulle__iovec_private_h__

/*
 * Internal helpers, not part of the public API. They are exposed here so
 * that the test suite can verify argument-validation behavior directly.
 */

#include "mulle-iovec.h"

int      mulle_iovec_validate_arguments( struct mulle_iovec *iov, int iovcnt);
size_t   mulle_iovec_get_length( struct mulle_iovec *iov, int iovcnt);

#endif
