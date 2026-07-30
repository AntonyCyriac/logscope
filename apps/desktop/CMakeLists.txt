# ==========================================================
# LogScope Desktop (Qt Widgets) — M14
# ==========================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

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

    # AGL.framework was removed from recent macOS SDKs; some Qt builds still reference it.
    find_library(LOGSCOPE_AGL_FRAMEWORK AGL)
    if(NOT LOGSCOPE_AGL_FRAMEWORK)
        target_link_options(logscope-desktop PRIVATE "-Wl,-weak_framework,AGL")
    endif()
endif()

target_include_directories(logscope-desktop
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../cli
)

if(WIN32)
    set_target_properties(logscope-desktop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS logscope-desktop RUNTIME DESTINATION bin)
