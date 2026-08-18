# ==========================================================
# LogScope Desktop (Qt Widgets) — M14
# ==========================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(APPLE)
    # Stub target: Qt may list "AGL" as a link-library name on older 6.8.x builds.
    if(NOT TARGET AGL)
        add_library(AGL INTERFACE IMPORTED GLOBAL)
    endif()
    # Prefer LogScope FindWrapOpenGL.cmake (no AGL) over Qt's copy.
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/qt-macos")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(logscope-desktop
    main.cpp
    main_window.cpp
    log_table_model.cpp
    theme_manager.cpp
    tail_worker.cpp
    timeline_panel.cpp
    timeline_load_worker.cpp
    crash_panel.cpp
    crash_load_worker.cpp
    pstack_text_utils.cpp
    analytics_panel.cpp
    ai_panel.cpp
    export_report_dialog.cpp
    configuration_editor_dialog.cpp
    open_log_dialog.cpp
    save_session_dialog.cpp
    run_stats_dialog.cpp
    ../cli/report_writer.cpp
    ../cli/stats_output.cpp
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

add_subdirectory(tests)
