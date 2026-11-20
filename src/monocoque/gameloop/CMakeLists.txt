set(gameloop_source_files
  gameloop.c
  gameloop.h
  loopdata.h
  testlooplua.h
  testlooplua.c
)

add_library(gameloop STATIC ${gameloop_source_files})

set_property(TARGET gameloop PROPERTY POSITION_INDEPENDENT_CODE ON)
