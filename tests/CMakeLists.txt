# Automated CI test suite — only built/run when ENABLE_TESTS is on (see
# root CMakeLists.txt). Manual, interactive, and hardware-dependent scratch
# tools live in tests/manual/ instead and are never built by this file.

add_executable(confighelper_test confighelper_test.c)
# helper (confighelper.c specifically) references slog/xml2/simulatorapi
# symbols elsewhere in the same translation unit — static libs resolve per
# object file, so all of these are needed even though this test only
# exercises strtodevsubsubtype.
target_link_libraries(confighelper_test helper config slog xml2 simulatorapi m ${PROC_LIBS})
add_test(NAME confighelper_test COMMAND confighelper_test)

add_executable(ensure_simd_test ensure_simd_test.c)
target_link_libraries(ensure_simd_test helper config slog xml2 simulatorapi m ${PROC_LIBS})
add_test(NAME ensure_simd_test COMMAND ensure_simd_test)

find_program(BASH_PROGRAM bash)
if (BASH_PROGRAM)
    add_test(NAME startup_play_missing_simd
        COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/startup_play.sh missing $<TARGET_FILE:monocoque-cli>)
    add_test(NAME startup_play_starts_simd
        COMMAND ${BASH_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/startup_play.sh starts $<TARGET_FILE:monocoque-cli>)
endif()
