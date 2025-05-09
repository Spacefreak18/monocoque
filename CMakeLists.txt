
# minimum CMake version required for C++20 support, among other things
cmake_minimum_required(VERSION 3.18)

# detect if Monocoque is being used as a sub-project of another CMake project
if(NOT DEFINED PROJECT_NAME)
    set(MONOCOQUE_SUBPROJECT OFF)
else()
    set(MONOCOQUE_SUBPROJECT ON)
endif()


SET_SOURCE_FILES_PROPERTIES( src/monocoque.c PROPERTIES LANGUAGE C)
#set(CMAKE_BUILD_TYPE Debug)


project(monocoque)

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed -ldl")
set(LIBUSB_INCLUDE_DIR /usr/include/libusb-1.0)
set(LIBXML_INCLUDE_DIR /usr/include/libxml2)

FIND_PACKAGE(Lua 5.3 REQUIRED)
set(INCLUDE_DIRS ${LUA_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})

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

add_executable(monocoque src/monocoque/monocoque.c)

#if(USE_PULSEAUDIO)
#else()
#endif()
message("Using pulseaudio backend...")
add_compile_definitions(USE_PULSEAUDIO=true)
target_link_libraries(monocoque m hidapi-hidraw pulse serialport xml2 argtable2 config gameloop helper devices slog simulatorapi uv xdg-basedir ${LUA_LIBRARY})

target_include_directories(monocoque PUBLIC config ${LIBXML_INCLUDE_DIR})

add_subdirectory(src/monocoque/gameloop)
add_subdirectory(src/monocoque/simulatorapi)
add_subdirectory(src/monocoque/helper)
add_subdirectory(src/monocoque/devices)
add_subdirectory(src/monocoque/slog)

#add_executable(listusb tests/testlibusb.c)
#target_include_directories(listusb PUBLIC)
#target_link_libraries(listusb portaudio hidapi-hidraw)
#add_test(listusb list-usb-devices listusb)

#add_executable(testrevburner tests/testrevburner.c)
#target_include_directories(testrevburner PUBLIC)
#target_link_libraries(testrevburner hidapi-hidraw)
#add_test(testrevburner testrevburner)

#add_executable(listsound tests/pa_devs.c)
#target_include_directories(listsound PUBLIC)
#target_link_libraries(listsound m portaudio)
#add_test(list-sound-devices listsound)
#
#add_executable(longsine tests/patest_longsine.c)
#target_include_directories(longsine PUBLIC ${LIBXML_INCLUDE_DIR})
#target_link_libraries(longsine m portaudio)
#add_test(longsine longsine)
#
#add_executable(parserevburnerxml tests/revburnerparsetest.c)
#target_include_directories(parserevburnerxml PUBLIC ${LIBXML_INCLUDE_DIR})
#target_link_libraries(parserevburnerxml portaudio xml2)
#add_test(parserevburnerxml parserevburnerxml)
#
add_executable(setmem tests/setmem.c)
target_include_directories(setmem PUBLIC)
target_link_libraries(setmem)
add_test(setmem setmem)

add_executable(getmem tests/getmem.c)
target_include_directories(getmem PUBLIC)
target_link_libraries(getmem)
add_test(getmem getmem)
#
#add_executable(setsimdata tests/setsimdata.c)
#target_include_directories(setsimdata PUBLIC)
#target_link_libraries(setsimdata)
#add_test(setsimdata setsimdata)
#
#add_executable(hidtest tests/hidtest.c)
#target_include_directories(hidtest PUBLIC)
#target_link_libraries(hidtest hidapi-hidraw)
#add_test(hidtest hidtest)
#
add_executable(simlighttest tests/simlighttest.c)
target_include_directories(simlighttest PUBLIC)
target_link_libraries(simlighttest serialport)
add_test(simlighttest simlighttest)


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
        target_compile_options(monocoque PRIVATE -fanalyzer -Wno-analyzer-possible-null-argument -Wno-analyzer-possible-null-dereference)
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
    add_subdirectory(tests)
    enable_testing()
endif()
