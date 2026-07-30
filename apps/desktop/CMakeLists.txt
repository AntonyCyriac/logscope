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
        scope_ai
        scope_analysis
        scope_analytics
        scope_foundation
        scope_investigation
        scope_reporting
        scope_search
)

target_include_directories(logscope-desktop
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../cli
)

if(WIN32)
    set_target_properties(logscope-desktop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS logscope-desktop RUNTIME DESTINATION bin)
