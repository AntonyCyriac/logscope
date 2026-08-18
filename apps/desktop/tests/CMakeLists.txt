# ==========================================================
# Desktop GUI integration tests (Qt Test, headless)
# ==========================================================

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(logscope_desktop_tests
    main_window_test.cpp
    ../main_window.cpp
    ../log_table_model.cpp
    ../theme_manager.cpp
    ../tail_worker.cpp
    ../timeline_panel.cpp
    ../timeline_load_worker.cpp
    ../crash_panel.cpp
    ../crash_load_worker.cpp
    ../pstack_text_utils.cpp
    ../analytics_panel.cpp
    ../ai_panel.cpp
    ../export_report_dialog.cpp
    ../configuration_editor_dialog.cpp
    ../open_log_dialog.cpp
    ../save_session_dialog.cpp
    ../run_stats_dialog.cpp
    ../../cli/report_writer.cpp
    ../../cli/stats_output.cpp
)

target_link_libraries(logscope_desktop_tests
    PRIVATE
        Qt6::Widgets
        Qt6::Test
        scope_application
)

target_include_directories(logscope_desktop_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cli
)

add_test(
    NAME logscope_desktop_tests
    COMMAND logscope_desktop_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_tests_properties(logscope_desktop_tests PROPERTIES
    LABELS "logscope_desktop_tests"
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

add_executable(logscope_desktop_parity_test
    investigation_parity_test.cpp
    ../main_window.cpp
    ../log_table_model.cpp
    ../theme_manager.cpp
    ../tail_worker.cpp
    ../timeline_panel.cpp
    ../timeline_load_worker.cpp
    ../crash_panel.cpp
    ../crash_load_worker.cpp
    ../pstack_text_utils.cpp
    ../analytics_panel.cpp
    ../ai_panel.cpp
    ../export_report_dialog.cpp
    ../configuration_editor_dialog.cpp
    ../open_log_dialog.cpp
    ../save_session_dialog.cpp
    ../run_stats_dialog.cpp
    ../../cli/report_writer.cpp
    ../../cli/stats_output.cpp
)

target_link_libraries(logscope_desktop_parity_test
    PRIVATE
        Qt6::Widgets
        Qt6::Test
        scope_application
)

target_include_directories(logscope_desktop_parity_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cli
)

add_test(
    NAME logscope_desktop_parity_test
    COMMAND logscope_desktop_parity_test
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_tests_properties(logscope_desktop_parity_test PROPERTIES
    LABELS "logscope_desktop_parity_test"
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

# ==========================================================
# README desktop screenshot capture (headless, offscreen)
# ==========================================================

add_executable(logscope_capture_readme_desktop
    capture_readme_screenshot.cpp
    ../main_window.cpp
    ../log_table_model.cpp
    ../theme_manager.cpp
    ../tail_worker.cpp
    ../timeline_panel.cpp
    ../timeline_load_worker.cpp
    ../crash_panel.cpp
    ../crash_load_worker.cpp
    ../pstack_text_utils.cpp
    ../analytics_panel.cpp
    ../ai_panel.cpp
    ../export_report_dialog.cpp
    ../configuration_editor_dialog.cpp
    ../open_log_dialog.cpp
    ../save_session_dialog.cpp
    ../run_stats_dialog.cpp
    ../../cli/report_writer.cpp
    ../../cli/stats_output.cpp
)

target_link_libraries(logscope_capture_readme_desktop
    PRIVATE
        Qt6::Widgets
        scope_application
)

target_include_directories(logscope_capture_readme_desktop
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cli
)
