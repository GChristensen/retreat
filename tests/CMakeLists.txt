add_executable(retreat_core_smoke
    core_smoke.cpp
    stubs/system_stub.cpp
    stubs/MonitorFactory_stub.cpp
    stubs/StateWindowFactory_stub.cpp)
target_link_libraries(retreat_core_smoke PRIVATE retreat_core retreat_options)

add_test(NAME retreat_core_smoke COMMAND retreat_core_smoke)
