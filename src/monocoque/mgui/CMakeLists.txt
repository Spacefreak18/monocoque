

set(NAPPGUI_DEMO NO CACHE BOOL "" FORCE)
set(NAPPGUI_WEB False CACHE BOOL "Build NAppGUI WebView support.")
set(_saved_build_type ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE Release)
add_subdirectory(nappgui_src)
set(CMAKE_BUILD_TYPE ${_saved_build_type})


add_library(mgui STATIC ${gui_source_files} ${NAppGUI_SOURCE_DIR}/src)
set_property(TARGET mgui PROPERTY POSITION_INDEPENDENT_CODE ON)
add_dependencies(mgui gui osapp osgui draw2d geom2d core osbs sewer encode)
target_include_directories(mgui PUBLIC ${LIBXML_INCLUDE_DIR} ${NAppGUI_SOURCE_DIR}/src ${GTK3_INCLUDE_DIRS})
set_target_properties(mgui PROPERTIES LINKER_LANGUAGE C)

set(NAPPGUI_LIB_DIR ${CMAKE_BINARY_DIR}/Release/lib)
target_link_libraries(mgui PUBLIC 
    #gui osapp osgui draw2d geom2d core osbs sewer osapp encode
    ${NAPPGUI_LIB_DIR}/libgui.a
    ${NAPPGUI_LIB_DIR}/libosapp.a
    ${NAPPGUI_LIB_DIR}/libosgui.a
    ${NAPPGUI_LIB_DIR}/libdraw2d.a
    ${NAPPGUI_LIB_DIR}/libgeom2d.a
    ${NAPPGUI_LIB_DIR}/libcore.a
    ${NAPPGUI_LIB_DIR}/libosbs.a
    ${NAPPGUI_LIB_DIR}/libsewer.a
    ${NAPPGUI_LIB_DIR}/libosapp.a
    ${NAPPGUI_LIB_DIR}/libencode.a
    ${GTK3_LIBRARIES})
