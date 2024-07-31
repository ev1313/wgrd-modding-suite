include(LibFindMacros)

libfind_pkg_check_modules(LibEpoxy_PKGCONF epoxy)

find_path(LibEpoxy_INCLUDE_DIR
  NAMES epoxy/gl.h
  HINTS ${LibEpoxy_PKGCONF_INCLUDE_DIRS} /mingw64/ /mingw64/include/
  PATH_SUFFIXES LibEpoxy
)

find_library(LibEpoxy_LIBRARY
  NAMES epoxy
  HINTS ${LibEpoxy_PKGCONF_LIBRARY_DIRS} /mingw64/ /mingw64/lib/
)

set(LibEpoxy_PROCESS_INCLUDES LibEpoxy_INCLUDE_DIR)
set(LibEpoxy_PROCESS_LIBS LibEpoxy_LIBRARY)
set(LibEpoxy_VERSION ${LibEpoxy_PKGCONF_VERSION})
libfind_process(LibEpoxy)

