#ifndef mulle__iovec__windows_h__
#define mulle__iovec__windows_h__

#include "include.h"

#include <stddef.h>
#include <sys/types.h>


#ifdef _WIN32

struct mulle_iovec;

MULLE__IOVEC_GLOBAL
ssize_t
   mulle_readv_windows( int fd, struct mulle_iovec *iov, int iovcnt);

MULLE__IOVEC_GLOBAL
ssize_t
   mulle_writev_windows( int fd, struct mulle_iovec *iov, int iovcnt);

#endif

#endif
