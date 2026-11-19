# Windows platform layer: WTL UI, Win32 system services, factories.
# It is compiled once per artifact (retreat.exe, retreatlib.pyd) because
# PYTHON_MODULE changes a few UI code paths; the core is shared.

find_path(WTL_INCLUDE_DIR atlapp.h PATH_SUFFIXES wtl REQUIRED)

set(RETREAT_WIN32_MODULES
    SentinelClient.cppm
    StateStore.cppm
    ui/AboutDlg.cppm
    ui/AlertWindowAdapter.cppm
    ui/BreakWindow.cppm
    ui/DispatchWnd.cppm
    ui/InputLock.cppm
    ui/InputMonitor.cppm
    ui/LockWindowAdapter.cppm
    ui/MessageWindow.cppm
    ui/QuoteWindow.cppm
    ui/TimerWindow.cppm
    ui/TranslucentWindow.cppm
    ui/TrayIconImpl.cppm
    ui/gdi/BitmapBuffer.cppm
    ui/gdi/QuoteCard.cppm
    ui/gdi/Shapes.cppm
    ui/gdi/TextBuffer.cppm
    ui/settings/BreakScreenSettingsSection.cppm
    ui/settings/BreaksSettingsSection.cppm
    ui/settings/ExceptionsSettingsSection.cppm
    ui/settings/MainOptionsDlg.cppm
    ui/settings/QuotesSettingsSection.cppm
    ui/settings/ScheduleSettingsSection.cppm
    ui/settings/SectionDlg.cppm
    ui/settings/SettingsControls.cppm)

set(RETREAT_WIN32_SOURCES
    main.cpp
    dbgcout.cpp
    system_win32.cpp
    MonitorFactory_win32.cpp
    StateWindowFactory_win32.cpp
    ui/settings/ColorButton.cpp
    res/retreat.rc)

function(retreat_add_win32_ui target)
    target_sources(${target}
        PRIVATE ${RETREAT_WIN32_SOURCES}
        PRIVATE FILE_SET CXX_MODULES FILES ${RETREAT_WIN32_MODULES})
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/res
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/settings
        ${WTL_INCLUDE_DIR})
    target_compile_definitions(${target} PRIVATE _WINDOWS STRICT)
    target_link_libraries(${target} PRIVATE
        retreat_core retreat_options
        gdiplus comctl32 pathcch strmiids)
endfunction()

if(RETREAT_BUILD_APP)
    add_executable(retreat WIN32)
    retreat_add_win32_ui(retreat)
endif()

if(RETREAT_BUILD_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development.SABIModule)
    Python3_add_library(retreatlib MODULE USE_SABI 3)
    retreat_add_win32_ui(retreatlib)
    target_sources(retreatlib PRIVATE python_module.cpp)
    target_compile_definitions(retreatlib PRIVATE PYTHON_MODULE)
    set_target_properties(retreatlib PROPERTIES SUFFIX ".pyd")
    if(RETREAT_PYD_OUTPUT_DIR)
        set_target_properties(retreatlib PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${RETREAT_PYD_OUTPUT_DIR}"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "${RETREAT_PYD_OUTPUT_DIR}")
    endif()
endif()
