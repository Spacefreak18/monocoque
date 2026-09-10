# Automated CI test suite — only built/run when ENABLE_TESTS is on (see
# root CMakeLists.txt). Manual, interactive, and hardware-dependent scratch
# tools live in tests/manual/ instead and are never built by this file.

add_executable(confighelper_test confighelper_test.c)
# helper (confighelper.c specifically) references slog/xml2/simulatorapi
# symbols elsewhere in the same translation unit — static libs resolve per
# object file, so all of these are needed even though this test only
# exercises strtodevsubsubtype.
target_link_libraries(confighelper_test helper config slog xml2 simulatorapi m proc2)
add_test(NAME confighelper_test COMMAND confighelper_test)
