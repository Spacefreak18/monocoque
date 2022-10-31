set(helper_source_files
  parameters.c
  parameters.h
  dirhelper.c
  dirhelper.h
  confighelper.c
  confighelper.h
)

set(LIBXML_INCLUDE_DIR /usr/include/libxml2)
include_directories("." ${LIBXML_INCLUDE_DIR})

add_library(helper STATIC ${helper_source_files})
