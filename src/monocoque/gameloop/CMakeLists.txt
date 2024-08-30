set(gameloop_source_files
  gameloop.c
  gameloop.h
  tachconfig.c
  tachconfig.h
  loopdata.h
)

set(LIBXML_INCLUDE_DIR /usr/include/libxml2)
include_directories("." ${LIBXML_INCLUDE_DIR})

add_library(gameloop STATIC ${gameloop_source_files})
