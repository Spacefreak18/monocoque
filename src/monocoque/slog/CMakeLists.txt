set(slog_source_files
  slog.c
  slog.h
)

add_library(slog STATIC ${slog_source_files})

set_property(TARGET slog PROPERTY POSITION_INDEPENDENT_CODE ON)
