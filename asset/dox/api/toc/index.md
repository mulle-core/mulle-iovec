# mulle-iovec Library Documentation for AI
<!-- Keywords: scatter-gather, iovec, readv, writev, portability, syscall, emulation -->

## 1. Introduction & Purpose

`mulle-iovec` is a cross-platform compatibility layer for POSIX scatter/gather
I/O — i.e. `readv(2)` / `writev(2)`. It provides a uniform way to read from or
write to a file descriptor across many buffers in a single call, on platforms
where the system call exists (POSIX/Unix/macOS/Linux), where it has a different
name (Windows emulates it via `ReadFile`/`WriteFile`), and where there is no
native scatter/gather support at all (a `read()`/`write()` loop emulation).

What problem does it solve? Code written against a single `struct mulle_iovec`
and `mulle_readv`/`mulle_writev` porters cleanly to all supported platforms
without preprocessor conditionals, while the underlying data layout is verified
(structurally identical to the system `struct iovec`) so the POSIX native path
costs nothing extra.

Key features at a high level:

- A tiny, ABI-compatible `struct mulle_iovec` (mirrors the system `struct iovec`).
- `mulle_readv`/`mulle_writev` delivered as `static inline` wrappers over the
  native syscall, or as real functions for emulated platforms.
- `mulle_writev_all` for "write all buffers or fail" semantics (retries short
  writes and `EINTR`, never returns a short count for a success).
- Hardened argument validation (`EINVAL` on bad `iovcnt`, NULL `iov`, or
  aggregate-length overflow) matching native `readv`/`writev` semantics.
- Version query helpers.

Relationship to other libraries: a foundational, small utility library of the
`mulle-core` family. It depends on `mulle-core` (which supplies `mulle-c11`
glue and the `MULLE_C_GLOBAL` symbol-export macro).

## 2. Key Concepts & Design Philosophy

- **One struct, one contract.** Everything centers on
  `struct mulle_iovec { void *iov_base; size_t iov_len; }`. Callers build an
  array (or "vector") of these buffers and pass it plus a count to the I/O
  functions.
- **Native when possible, emulate otherwise.** The header picks one of three
  backends with preprocessor macros:
  - `MULLE_IOVEC_NATIVE` on `__unix__` / `__APPLE__` / `__linux__`: `mulle_readv`
    and `mulle_writev` are `static inline` calls straight into the system
    `readv`/`writev`, casting the iovec array. Five `_Static_assert`s guarantee
    `struct mulle_iovec` is memory-compatible with `struct iovec`, so the cast
    is safe and no copying occurs.
  - `MULLE_IOVEC_WINDOWS` on `_WIN32`: dispatches to `mulle_readv_windows` /
    `mulle_writev_windows`.
  - `MULLE_IOVEC_EMULATED` otherwise: real exported functions that loop over
    `read()`/`write()` per buffer.
- **readv/writev semantics are honored.** Reads stop early when a buffer is not
  fully filled; writes stop at the first short write (later buffers are treated
  as untouched when an earlier one was short); `EINTR` is retried internally so
  partial byte counts are not lost.
- **Validation mirrors the native syscall.** Invalid `iovcnt` (< 0 or > IOV_MAX),
  a NULL `iov` with a positive count, or an aggregate length exceeding `SSIZE_MAX`
  fail with `EINVAL` — before any I/O happens.
- On Windows, buffer alignment cannot be assumed, so `ReadFileScatter`/
  `WriteFileGather` (which require page-aligned buffers and overlapped handles)
  are avoided in favor of a correct loop of `ReadFile`/`WriteFile`.

## 3. Core API & Data Structures

All signatures below are copied verbatim from the public headers.

### 3.1. `mulle-iovec.h`

#### `struct mulle_iovec`
- **Purpose:** The central data type. Describes one contiguous memory buffer for
  scatter/gather I/O. Its layout is verified to be identical to the platform
  `struct iovec` on POSIX systems.
- **Key Fields:**
  - `void *iov_base` — pointer to the buffer.
  - `size_t iov_len` — length of the buffer in bytes.

#### Backend selection macros
- `MULLE_IOVEC_NATIVE` — defined on `__unix__` / `__APPLE__` / `__linux__`;
  the inline syscall wrapper path is used.
- `MULLE_IOVEC_WINDOWS` — defined on `_WIN32`; the Windows emulation path is used.
- `MULLE_IOVEC_EMULATED` — defined when neither applies; the loop-based
  emulation path is used.

#### `mulle_readv`
```c
static inline ssize_t
   mulle_readv( int fd, struct mulle_iovec *iov, int iovcnt)
```
- **Purpose:** Scatter-read: fills up to `iovcnt` buffers from the fd in order.
  Equivalent to `readv(2)`; on native platforms it *is* `readv(2)`.
- **Return:** number of bytes read; `-1` on error (with `errno` set). A short
  read is possible (e.g. fewer bytes than requested available, or on handled
  partial reads).

#### `mulle_writev`
```c
static inline ssize_t
   mulle_writev( int fd, struct mulle_iovec *iov, int iovcnt)
```
- **Purpose:** Gather-write: writes the `iovcnt` buffers in order to the fd.
  Equivalent to `writev(2)`; on native platforms it *is* `writev(2)`.
- **Return:** number of bytes written; `-1` on error (with `errno` set). May be
  a short count; stops at the first short write — later buffers are not written.

#### `mulle_writev_all`
```c
static inline ssize_t
   mulle_writev_all( int fd, struct mulle_iovec *iov, int iovcnt)
```
- **Purpose:** Write every buffer completely, or fail. Unlike `mulle_writev`,
  it never returns a short count of success: short writes are consumed and
  retried internally, and interrupted operations (`EINTR`) are retried.
- **Mutates the input array while working:** as short writes are absorbed, the
  remaining leading iovec entries are advanced (`iov_base`/`iov_len` adjusted).
- **Return:** total number of bytes written across all buffers; `-1` on error.
  If an error occurs after some bytes were already written, it returns the
  number of bytes written so far (not `-1`).

#### Version functions
```c
static inline unsigned int   mulle_iovec_get_version_major( void)
static inline unsigned int   mulle_iovec_get_version_minor( void)
static inline unsigned int   mulle_iovec_get_version_patch( void)
MULLE__IOVEC_GLOBAL
uint32_t   mulle_iovec_get_version( void);
```
- **Purpose:** Query the library version. `MULLE__IOVEC_VERSION` is the packed
  `(major << 20) | (minor << 8) | patch` value; the inline helpers decode the
  major, minor and patch parts; `mulle_iovec_get_version` returns the packed
  value at runtime.

### 3.2. `mulle-iovec-windows.h` (Windows only)

Header behind the `MULLE_IOVEC_WINDOWS` path; declarations are guarded by
`#ifdef _WIN32`.

```c
MULLE__IOVEC_GLOBAL
ssize_t
   mulle_readv_windows( int fd, struct mulle_iovec *iov, int iovcnt);

MULLE__IOVEC_GLOBAL
ssize_t
   mulle_writev_windows( int fd, struct mulle_iovec *iov, int iovcnt);
```
- **Purpose:** Windows backends for `mulle_readv`/`mulle_writev`. Convert the
  POSIX fd with `_get_osfhandle`, validate like native `readv`/`writev` (also
  rejecting any single buffer longer than `MAXDWORD`), then loop `ReadFile` /
  `WriteFile` per buffer, mapping Win32 errors to POSIX `errno`
  (`EBADF`, `EACCES`, `EPIPE`, `ENOSPC`, `EIO`). Partial progress is reported
  before the error if any bytes were transferred.
- These are exported symbols; callers normally use `mulle_readv`/`mulle_writev`.

### 3.3. `mulle-iovec-private.h` (internal; exposed for testing)

Not part of the public API contract, but declared in a header so the test suite
can exercise them:

```c
int      mulle_iovec_validate_arguments( struct mulle_iovec *iov, int iovcnt);
size_t   mulle_iovec_get_length( struct mulle_iovec *iov, int iovcnt);
```
- `mulle_iovec_validate_arguments` — shared argument validation used by the
  emulated and Windows backends. Returns `0` on success; sets `errno = EINVAL`
  and returns `-1` if `iovcnt` is negative or over `IOV_MAX`, `iov` is NULL with
  a positive count, or the sum of `iov_len` overflows `SSIZE_MAX`.
- `mulle_iovec_get_length` — returns the aggregate length of the iovec array,
  or `(size_t) -1` with `errno = EINVAL` under the same failure conditions.

## 4. Performance Characteristics

- **Native platforms:** zero-overhead. `mulle_readv`/`mulle_writev` are
  `static inline` and reduce to a single `readv`/`writev` syscall; the iovec
  cast is a no-op verified at compile time by `_Static_assert`.
- **Windows:** N syscalls (one `ReadFile`/`WriteFile` per buffer) instead of one
  `WSARecv`/`WSASend`; the trade-off buys universal correctness for arbitrary
  buffer alignment and non-overlapped handles.
- **Emulated platforms:** N `read()`/`write()` syscalls, one per buffer. This is
  the slowest backend and is only for barebones systems.
- **`mulle_writev_all`:** one syscall per write attempt on native platforms (the
  iovec array is advanced in memory as short writes are absorbed); worst case
  more syscalls than `mulle_writev` because short writes are retried.
- **Validation cost:** O(iovcnt) scan of the array to validate and (for the
  helper) sum the lengths before performing I/O.
- **Thread safety:** no shared mutable state; functions are thread-safe as long
  as the caller's iovec array and fd are not concurrently modified. The fd must
  be on a descriptor that supports the underlying read/write operation.
- **Scope:** regular files and pipes everywhere. On POSIX, `int` fds happen to
  accept sockets, but sockets are not part of the contract on any platform — on
  Windows use `WSARecv`/`WSASend` directly.

## 5. AI Usage Recommendations & Patterns

- **Best Practices**
  - Initialize every `struct mulle_iovec` entry's `iov_base` and `iov_len`
    before calling; garbage lengths produce garbage I/O.
  - Prefer `mulle_writev_all` when you need the whole vector delivered
    (e.g. writing a protocol message) and you cannot tolerate a short write.
    Use plain `mulle_writev` when a short count is a meaningful signal.
  - Always check the return value: `-1` with `errno` for errors, otherwise the
    byte count. With `mulle_writev_all`, a non-`-1` return that is less than
    the aggregate length implies an error occurred mid-write.
  - Let `mulle_readv`/`mulle_writev` select the backend for you; do not call
    the `_windows` variants or branch on `MULLE_IOVEC_*` yourself unless you are
    implementing platform-specific behavior.
  - Include `<mulle-iovec/mulle-iovec.h>`; do not include the reflect-generated
    or private headers in application code.
- **Common Pitfalls**
  - Do not mutate `iov` (or the buffers it points to) while a call is in
    flight. `mulle_writev_all` temporarily advances the entries of the array you
    pass it; your input array is not preserved if a short write occurs.
  - Do not rely on sockets working: the documented contract covers regular files
    and pipes only.
  - Do not pass `iovcnt == 0` with a NULL `iov` expecting success — use a valid
    pointer or ensure consistent (count, pointer) pairs; validation enforces
    the same rules as native `readv`/`writev`.
  - Do not compare `struct mulle_iovec` to other frameworks' iovec types for
    equality of semantics; only its layout is guaranteed compatible with the
    system `struct iovec` (and only on the native path).
  - Version macros/inlines are in the public header — do not hardcode version
    checks where the helpers are available.

## 6. Integration Examples

### Example 1: Scatter-write two strings, then scatter-read them back

Demonstrates the standard lifecycle: build an iovec array, write it, and read
it back with `mulle_readv`. Mirrors the project test.

```c
#include <mulle-iovec/mulle-iovec.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


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

   // Setup write iovec
   write_iov[0].iov_base = str1;
   write_iov[0].iov_len  = strlen( str1);
   write_iov[1].iov_base = str2;
   write_iov[1].iov_len  = strlen( str2);

   // Create temporary file
   fd = open( "/tmp/test_iovec.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
   if( fd == -1)
      return( 1);

   // Write using mulle_writev
   bytes_written = mulle_writev( fd, write_iov, 2);
   close( fd);
   printf( "Bytes written: %zd\n", bytes_written);

   // Setup read iovec
   memset( buffer1, 0, sizeof( buffer1));
   memset( buffer2, 0, sizeof( buffer2));
   read_iov[0].iov_base = buffer1;
   read_iov[0].iov_len  = strlen( str1);
   read_iov[1].iov_base = buffer2;
   read_iov[1].iov_len  = strlen( str2);

   // Read back using mulle_readv
   fd = open( "/tmp/test_iovec.txt", O_RDONLY);
   if( fd == -1)
      return( 1);

   bytes_read = mulle_readv( fd, read_iov, 2);
   close( fd);
   printf( "Bytes read: %zd\n", bytes_read);
   printf( "Read content: '%s%s'\n", buffer1, buffer2);

   unlink( "/tmp/test_iovec.txt");

   return( 0);
}
```

### Example 2: Writing a whole message with `mulle_writev_all`

Use `mulle_writev_all` when a short write is not acceptable and each buffer in
the vector must be fully written. It retries short writes and `EINTR`
internally and returns the total bytes written.

```c
#include <mulle-iovec/mulle-iovec.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>


static int
   write_all_to( int fd, char *a, char *b, size_t alen, size_t blen)
{
   struct mulle_iovec   iov[2];
   ssize_t              bytes;

   iov[0].iov_base = a;
   iov[0].iov_len  = alen;
   iov[1].iov_base = b;
   iov[1].iov_len  = blen;

   bytes = mulle_writev_all( fd, iov, 2);
   if( bytes == -1)
      return( -1);              /* error, errno set */
   if( bytes != (ssize_t)( alen + blen))
      return( -1);              /* partial write before an error */

   return( 0);
}


int   main( void)
{
   char   a[]   = "The quick brown fox ";
   char   b[]   = "jumps over the lazy dog.\n";
   int    fd;

   fd = open( "/tmp/out.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
   if( fd == -1)
      return( 1);

   if( write_all_to( fd, a, b, strlen( a), strlen( b)))
   {
      close( fd);
      return( 1);
   }

   close( fd);
   return( 0);
}
```

## 7. Dependencies

Direct `mulle-sde` library dependencies (from `.mulle/etc/sourcetree/config`):

- `mulle-core` — provides `MULLE_C_GLOBAL` (used for symbol export) and the
  `mulle-c11` convenience includes needed at build time.

## 8. Shortcut

This is the initial `index.md` for this project (`asset/dox/api/toc/index.md`
did not previously exist), so it documents the full current API as of commit
`2aae859` (which includes the `mulle_writev_all` feature and the hardened
readv/writev emulation semantics from `15dc2de`).