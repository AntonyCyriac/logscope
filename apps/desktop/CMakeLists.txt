# ==========================================================
# LogScope Desktop (Qt Widgets) — M14
# ==========================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# AGL.framework was removed from macOS 14+ / Tahoe SDKs; some Qt builds still inject it
# via WrapOpenGL::WrapOpenGL (QTBUG-137687). Filter it before linking the desktop target.
if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(_logscope_qt_opengl_libs WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if(_logscope_qt_opengl_libs)
        list(FILTER _logscope_qt_opengl_libs EXCLUDE REGEX "AGL")
        set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES
            INTERFACE_LINK_LIBRARIES "${_logscope_qt_opengl_libs}")
    endif()
endif()

add_executable(logscope-desktop
    main.cpp
    main_window.cpp
    log_table_model.cpp
    theme_manager.cpp
    tail_worker.cpp
    analytics_panel.cpp
    ai_panel.cpp
    ../cli/report_writer.cpp
)

target_link_libraries(logscope-desktop
    PRIVATE
        Qt6::Widgets
        scope_application
)

if(APPLE)
    set_target_properties(logscope-desktop PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "LogScope"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.logscope.desktop"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )

    # Fallback when Qt or the SDK still references AGL after the filter above.
    target_link_options(logscope-desktop PRIVATE "-Wl,-weak_framework,AGL")
endif()

target_include_directories(logscope-desktop
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../cli
)

if(WIN32)
    set_target_properties(logscope-desktop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS logscope-desktop
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
