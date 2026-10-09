set(helper_source_files
  parameters.c
  parameters.h
  dirhelper.c
  dirhelper.h
  confighelper.c
  confighelper.h
  ensure_simd.c
  ensure_simd.h
)

set(LIBXML_INCLUDE_DIR /usr/include/libxml2)
include_directories("." ${LIBXML_INCLUDE_DIR})

add_library(helper STATIC ${helper_source_files})

set_property(TARGET helper PROPERTY POSITION_INDEPENDENT_CODE ON)
