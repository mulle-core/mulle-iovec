# mulle-iovec

#### 🧺 iovec compatiblity layer

# mulle-iovec

#### 🧺 iovec compatibility layer

It's supposed to provide a uniform way of doing scatter/gather I/O on various
platforms. Windows provides scatter/gather but under different names. For
very barebones system, there is an emulation using read/write.



| Release Version                                       | Release Notes  | AI Documentation
|-------------------------------------------------------|----------------|---------------
| ![Mulle kybernetiK tag](https://img.shields.io/github/tag/mulle-core/mulle-iovec.svg) [![Build Status](https://github.com/mulle-core/mulle-iovec/workflows/CI/badge.svg)](//github.com/mulle-core/mulle-iovec/actions) | [RELEASENOTES](RELEASENOTES.md) | [DeepWiki for mulle-iovec](https://deepwiki.com/mulle-core/mulle-iovec)






### You are here

![Overview](overview.dot.svg)





## Add

Use [mulle-sde](//github.com/mulle-sde) to add mulle-iovec to your project:

``` sh
mulle-sde add github:mulle-core/mulle-iovec
```

To only add the sources of mulle-iovec with dependency
sources use [clib](https://github.com/clibs/clib):


``` sh
clib install --out src/mulle-core mulle-core/mulle-iovec
```

Add `-isystem src/mulle-core` to your `CFLAGS` and compile all the sources that were downloaded with your project.


## Install

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-iovec and all dependencies:

``` sh
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-core/mulle-iovec/archive/latest.tar.gz
```

### Legacy Installation

#### Requirements

Preferably install mulle-core and be done with it:

| Requirements                                     | Description
|--------------------------------------------------|-----------------------
| [mulle-core](//github.com/mulle-core/mulle-core) |🌋 Almagamated library of mulle-core + mulle-concurrent + mulle-c


Or if you really want to do it exhaustively:

| Requirements                                 | Description
|----------------------------------------------|-----------------------
| [mulle-c11](https://github.com/mulle-c/mulle-c11)             | 🔀 Cross-platform C compiler glue (and some cpp conveniences)

#### Download & Install

Download the latest [tar](https://github.com/mulle-core/mulle-iovec/archive/refs/tags/latest.tar.gz) or [zip](https://github.com/mulle-core/mulle-iovec/archive/refs/tags/latest.zip) archive and unpack it.

Install **mulle-iovec** into `/usr/local` with [cmake](https://cmake.org):

``` sh
PREFIX_DIR="/usr/local"
cmake -B build                               \
      -DMULLE_SDK_PATH="${PREFIX_DIR}"       \
      -DCMAKE_INSTALL_PREFIX="${PREFIX_DIR}" \
      -DCMAKE_PREFIX_PATH="${PREFIX_DIR}"    \
      -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release &&
cmake --install build --config Release
```


## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



