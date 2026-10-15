## 0.8.0


feature: add Windows-only transfer-size testing hook and public API docs

* document ``mulle_iovec_windows_set_max_transfer`,` a Windows-only hook that caps bytes per `ReadFile`/`WriteFile` transfer (values of `0` or above `MAXDWORD` reset to `MAXDWORD`)
* annotate the non-public ``mulle_iovec_validate_arguments`` and ``mulle_iovec_get_length`` test APIs with ``MULLE__IOVEC_GLOBAL`` for visibility control


feature: support iovec entries larger than 4 GB on Windows

* Windows readv/writev now split a single iovec longer than MAXDWORD into several ReadFile/WriteFile transfers instead of failing with EINVAL
* writev stops at the first short transfer, so buffers after a partial write are left untouched (POSIX semantics)
* transfer size is capped for reads and writes alike, matching the short-count behavior POSIX applies at `MAX_RW_COUNT`


### 0.7.56

Various small improvements
