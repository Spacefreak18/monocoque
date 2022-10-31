set(simulatorapi_source_files
  simmapper.c
  simmapper.h
  simdata.h
  test.h
  simapi/acdata.h
  ac.h
)

add_library(simulatorapi STATIC ${simulatorapi_source_files})
