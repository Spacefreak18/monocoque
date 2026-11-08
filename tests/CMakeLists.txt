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

# Module-focused hardware-free device unit tests. Each target compiles exactly
# one production translation unit and substitutes external I/O dependencies.
pkg_check_modules(CMOCKA REQUIRED IMPORTED_TARGET cmocka)

function(add_driver_test name source)
    string(REPLACE "/" "_" target "${name}")
    set(target "${target}_test")
    add_executable(${target} devices/${name}_test.c
        ${PROJECT_SOURCE_DIR}/src/monocoque/devices/${source})
    target_link_libraries(${target} PRIVATE PkgConfig::CMOCKA slog m Lua::Lua)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND UNIX AND NOT APPLE)
        target_compile_options(${target} PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(${target} PRIVATE -Wl,--gc-sections)
    endif()
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES LABELS "unit;devices")
endfunction()

add_driver_test(serial/arduino serial/arduino.c)
add_driver_test(serial/moza serial/moza.c)
add_driver_test(serial/moza_new serial/moza_new.c)
add_driver_test(serial/moza_ks_pro_wheel serial/moza_ks_pro_wheel.c)
add_driver_test(usb/revburner usb/revburner.c)
add_driver_test(usb/simnetpedals usb/simnetpedals.c)
add_driver_test(usb/logitechg29 usb/wheels/logitechg29.c)
add_driver_test(usb/cammusc5 usb/wheels/cammusc5.c)
add_driver_test(usb/cammusc12 usb/wheels/cammusc12.c)
add_driver_test(usb/simagicp1000 usb/simagicp1000.c)
add_driver_test(usb/cslelitev3 usb/cslelitev3.c)
add_driver_test(usb/gtneo usb/wheels/gtneo.c)
add_driver_test(sound/usb_generic_shaker_pulse sound/usb_generic_shaker_pulse.c)
add_driver_test(serial/arduinoledlua serial/arduinoledlua.c)
