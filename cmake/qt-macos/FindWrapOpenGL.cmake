# LogScope override for macOS Tahoe (SDK 26): AGL.framework was removed.
# Qt 6.8.x FindWrapOpenGL.cmake still adds -framework AGL (QTBUG-137687).
# Prepended to CMAKE_MODULE_PATH before find_package(Qt6) in apps/desktop.

if(TARGET WrapOpenGL::WrapOpenGL)
    set(WrapOpenGL_FOUND ON)
    return()
endif()

set(WrapOpenGL_FOUND OFF)

find_package(OpenGL ${WrapOpenGL_FIND_VERSION})

if(OpenGL_FOUND)
    set(WrapOpenGL_FOUND ON)

    add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)

    get_target_property(_logscope_opengl_fw_lib_path OpenGL::GL IMPORTED_LOCATION)
    if(_logscope_opengl_fw_lib_path AND NOT _logscope_opengl_fw_lib_path MATCHES "/([^/]+)\\.framework$")
        get_filename_component(_logscope_opengl_fw_path "${_logscope_opengl_fw_lib_path}" DIRECTORY)
    endif()

    if(NOT _logscope_opengl_fw_path)
        set(_logscope_opengl_fw_path "-framework OpenGL")
    endif()

    target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE ${_logscope_opengl_fw_path})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WrapOpenGL DEFAULT_MSG WrapOpenGL_FOUND)
