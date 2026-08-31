cmake_minimum_required(VERSION 3.18)
# detect if Monocoque is being used as a sub-project of another CMake project
if(NOT DEFINED PROJECT_NAME)
    set(MONOCOQUE_SUBPROJECT OFF)
else()
    set(MONOCOQUE_SUBPROJECT ON)
endif()


SET_SOURCE_FILES_PROPERTIES( src/monocoque.c PROPERTIES LANGUAGE C)
#set(CMAKE_BUILD_TYPE Debug)

option(BUILD_GUI "Build GUI support" OFF)
option(BUILD_SHARED "Build monocoque shared library" OFF)
option(ENABLE_TESTS "Build test suite" OFF)


project(monocoque)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed -ldl")
set(LIBUSB_INCLUDE_DIR /usr/include/libusb-1.0)
set(LIBXML_INCLUDE_DIR /usr/include/libxml2)

FIND_PACKAGE(Lua)
set(INCLUDE_DIRS ${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
if(Lua_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )

endif()
message("Lua ${Lua_VERSION} found")
if (Lua_VERSION VERSION_GREATER_EQUAL "5.5.0")
    add_compile_definitions(USE_LUA_55)
else()
    remove_definitions(USE_LUA_55)
endif()

#FIND_PATH(LIBUSB_INCLUDE_DIR libusb.h
#	HINTS $ENV{LIBUSB_ROOT}
#	PATHS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS}
#	PATH_SUFFIXES include)
#
#FIND_LIBRARY(LIBUSB_LIBRARY NAMES usb-1.0
#	HINTS $ENV{LIBUSB_ROOT}
#	PATHS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS}
#	PATH_SUFFIXES lib)
#
set(HIDAPI_WITH_LIBUSB FALSE) # surely will be used only on Linux
set(BUILD_SHARED_LIBS TRUE) # HIDAPI as static library on all platforms

if(BUILD_SHARED)
    add_library(monocoque SHARED src/monocoque/monocoque.c)
else()
    add_library(monocoque STATIC src/monocoque/monocoque.c)
endif()
add_executable(monocoque-cli src/monocoque/monocoque-cli.c)
set_target_properties(monocoque-cli PROPERTIES OUTPUT_NAME monocoque)
if(BUILD_GUI)
    add_executable(monocoque-gui src/monocoque/monocoque-gui.c)
    set_target_properties(monocoque-gui PROPERTIES OUTPUT_NAME gmonocoque)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPROCPS libprocps)
pkg_check_modules(LIBPROC2 libproc2)

if (LIBPROCPS_FOUND)
    #add_compile_definitions(USE_OLD_PID_VAL=0)
elseif (LIBPROC2_FOUND)
    # Some distro packagings (e.g. the freedesktop Flatpak SDK) ship a
    # libproc2.pc with `Version: UNKNOWN` rather than a real version
    # string. VERSION_GREATER_EQUAL against that always evaluates false,
    # silently falling through to the old (pre-4.0.5) PIDS_VAL API and
    # failing to compile against an actually-current libproc2 whose real
    # header has already moved to the new 3-argument macro. Only take the
    # old-API branch when the version string is genuinely parseable and
    # actually old.
    if (LIBPROC2_VERSION MATCHES "^[0-9]" AND LIBPROC2_VERSION VERSION_LESS "4.0.5")
        add_compile_definitions(USE_OLD_PID_VAL=1)
    endif()
else()
    message(FATAL_ERROR "Either libprocps or libproc2 is required")
endif()

#if(USE_PULSEAUDIO)
#else()
#endif()

message("Using pulseaudio backend...")
add_compile_definitions(USE_PULSEAUDIO=true)

add_subdirectory(src/monocoque/gameloop)
add_subdirectory(src/monocoque/simulatorapi)
add_subdirectory(src/monocoque/helper)
add_subdirectory(src/monocoque/devices)
add_subdirectory(src/monocoque/slog)

target_link_libraries(monocoque m hidapi-hidraw pulse serialport xml2 argtable2 config gameloop helper devices slog simulatorapi uv xdg-basedir Lua::Lua proc2)
target_include_directories(monocoque PUBLIC config ${LIBXML_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
target_link_libraries(monocoque-cli m hidapi-hidraw pulse serialport xml2 argtable2 config gameloop helper devices slog simulatorapi uv xdg-basedir Lua::Lua proc2)
target_include_directories(monocoque-cli PUBLIC config ${LIBXML_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
if(BUILD_GUI)
    add_subdirectory(src/monocoque/mgui)
    target_link_libraries(monocoque-gui m hidapi-hidraw pulse serialport xml2 argtable2 config mgui gameloop helper devices slog simulatorapi uv xdg-basedir Lua::Lua proc2)
    target_include_directories(monocoque-gui PUBLIC config ${LIBXML_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
endif()




# used for enabling additional compiler options if supported
include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    message(STATUS "[monocoque] Checking if compiler supports warning flag '${flag}'")
    check_cxx_compiler_flag("${flag}" flag_supported)
    if(flag_supported)
        message(STATUS "[monocoque] Enabling warning flag '${flag}'")
        target_compile_options(monocoque INTERFACE "${flag}")
    endif()
    unset(flag_supported CACHE)
endfunction()

# enable a large amount of extra warnings, regardless of build mode
if (MSVC) # MSVC supports different warning options to GCC/Clang
    enable_cxx_compiler_flag_if_supported("/W3")  # set warning level 3
    # if tests are enabled, enable converting all warnings to errors too
    if (ENABLE_TESTS)
        # add_compile_options(/WX)
        enable_cxx_compiler_flag_if_supported("/WX")
    endif()
else() # GCC/Clang warning option
    # NOTE: GCC and Clang support most of the same options, but neither supports all
    # of the others'. By only enabling them if supported, we get graceful failure
    # when trying to enable unsupported flags
    # e.g. at the time of writing, GCC does not support -Wdocumentation
    #
    # enable all warnings about 'questionable constructs'

    if(analyze)
        message("-- Analyzer is on")
        target_compile_options(monocoque PRIVATE -fanalyzer -Wno-analyzer-possible-null-argument -Wno-analyzer-possible-null-dereference -fsanitize=address -g -fno-omit-frame-pointer)
    endif()

    enable_cxx_compiler_flag_if_supported("-Wall")
    # issue 'pedantic' warnings for strict ISO compliance
    enable_cxx_compiler_flag_if_supported("-pedantic")
    # enable 'extra' strict warnings
    enable_cxx_compiler_flag_if_supported("-Wextra")
    # enable sign conversion warnings
    enable_cxx_compiler_flag_if_supported("-Wsign-conversion")
    # enable warnings about mistakes in Doxygen documentation
    enable_cxx_compiler_flag_if_supported("-Wdocumentation")
    # if tests are enabled, enable converting all warnings to errors too
    if (ENABLE_TESTS)
        enable_cxx_compiler_flag_if_supported("-Werror")
        # exclude the following kinds of warnings from being converted into errors
        # unknown-pragma is useful to have as a warning but not as an error, if you have
        # pragmas which are for the consumption of one compiler only
        enable_cxx_compiler_flag_if_supported("-Wno-error=unknown-pragmas")
        # unused variable and function warnings are helpful but we don't need them as errors
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-function")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-variable")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-parameter")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-private-field")
        enable_cxx_compiler_flag_if_supported("-Wno-error=unused-but-set-variable")
    endif()
endif()

# library
# unit tests --only enable if requested AND we're not building as a sub-project
if(ENABLE_TESTS AND NOT MONOCOQUE_SUBPROJECT)
    message(STATUS "[monocoque] Unit Tests Enabled")
    enable_testing()
    add_subdirectory(tests)
endif()

# simd -- the telemetry daemon everything else here reads from. simapi ships
# its own CMakeLists with a BUILD_SIMD option, but this tree never invokes it:
# src/monocoque/simulatorapi compiles simapi's sources straight into the static
# `simulatorapi` library instead. So the target is declared here over that same
# library, which also means the simd in a monocoque package is built from the
# exact simapi commit monocoque itself was built against, rather than whatever
# happens to be installed system-wide.
#
# Packaged alongside monocoque because monocoque does nothing without it: with
# no simd running, simapi_get_sim() finds no /dev/shm/SIMAPI.DAT and falls back
# to scanning /proc for the game itself.
option(BUILD_SIMD "Build the simd telemetry daemon alongside monocoque" ON)
if(BUILD_SIMD)
    set(SIMAPI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/monocoque/simulatorapi/simapi)
    add_executable(simd
        ${SIMAPI_SRC_DIR}/simd/simd.c
        ${SIMAPI_SRC_DIR}/simd/parameters.c
        ${SIMAPI_SRC_DIR}/simd/confighelper.c
        ${SIMAPI_SRC_DIR}/simd/dirhelper.c
        ${SIMAPI_SRC_DIR}/simd/poke.c
        # Not part of the simulatorapi library above, which only pulls in
        # mapacdata.c from simmap/.
        ${SIMAPI_SRC_DIR}/simmap/mapsimdata.c)
    # simd's own build prefers argtable3 and falls back to argtable2. monocoque
    # links argtable2 everywhere, so pin the same one rather than let two halves
    # of one package disagree about which argtable they need at runtime.
    # simd.c carries its own copy of is_pid_running(), and so does simapi's
    # getpid.c. Upstream links simd against libsimapi.so, where the executable's
    # definition simply wins; here simapi is a static archive and the linker
    # refuses the duplicate outright. Renaming simd's copy for its own
    # translation units keeps both definitions and leaves each caller bound to
    # the one it was written against -- no patch to the pinned submodule, and no
    # --allow-multiple-definition silently picking a winner.
    target_compile_definitions(simd PRIVATE HAVE_ARGTABLE2 is_pid_running=simd_is_pid_running)
    target_include_directories(simd PRIVATE
        ${SIMAPI_SRC_DIR}/simapi
        ${SIMAPI_SRC_DIR}/include
        ${SIMAPI_SRC_DIR}/simmap
        ${SIMAPI_SRC_DIR}/simd)
    target_link_libraries(simd m dl uv yder argtable2 config simulatorapi proc2)
    # orcania sits under yder. A shared libyder.so records that dependency and
    # pulls it in on its own, which is why Debian and Ubuntu need nothing here;
    # a static libyder.a records nothing, so every o_malloc/split_string it
    # calls comes back undefined. Fedora has no yder package at all and its rpm
    # job vendors both as static archives, so the link has to name orcania
    # explicitly -- after yder, which is the order a static link needs.
    find_library(ORCANIA_LIBRARY orcania)
    if(ORCANIA_LIBRARY)
        target_link_libraries(simd ${ORCANIA_LIBRARY})
    endif()
endif()

install(TARGETS monocoque-cli RUNTIME DESTINATION bin)
if(BUILD_GUI)
    install(TARGETS monocoque-gui RUNTIME DESTINATION bin)
endif()
if(BUILD_SIMD)
    install(TARGETS simd RUNTIME DESTINATION bin)
    # simd runs without ~/.config/simd/simd.config, but degraded: it logs
    # "Error with config file" then "Disabling Automatic Bridge Mode" and
    # carries on mapping telemetry (observed by running it with an empty home
    # bind-mounted over the real one). What it loses is the auto-bridge -- the
    # per-game launchexe/liveexe table that starts the Windows bridge under
    # Proton, which is how a sim's telemetry reaches /dev/shm at all. simapi's
    # own build installs this file straight into $HOME, which a package must
    # not do, so it ships as an example to copy. Note simd resolves that path
    # from getpwuid(), not $HOME.
    install(FILES ${SIMAPI_SRC_DIR}/simd/conf/simd.config DESTINATION share/monocoque)
endif()
install(FILES tools/monocoque.desktop DESTINATION share/applications)
