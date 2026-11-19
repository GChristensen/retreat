# sentinel.exe: restarts retreat.exe / Enso when they are terminated. Plain Win32,
# shares only the protocol header with the application.

add_executable(sentinel WIN32 main.cpp)
target_include_directories(sentinel PRIVATE ${CMAKE_SOURCE_DIR}/src/win32)
target_compile_definitions(sentinel PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(sentinel PRIVATE retreat_options shell32)

# the application looks for it next to retreat.exe / retreatlib.pyd
set_target_properties(sentinel PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/win32")

if(RETREAT_BUILD_PYTHON AND RETREAT_PYD_OUTPUT_DIR)
    add_custom_command(TARGET sentinel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:sentinel> "${RETREAT_PYD_OUTPUT_DIR}")
endif()
