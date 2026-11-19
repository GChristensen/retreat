# Portable core: scheduling, state machine, settings. No Windows dependencies;
# platform services (`system` module) and the factories are implemented by the
# platform layer (src/win32) or by test stubs (tests/).

find_package(Boost CONFIG REQUIRED COMPONENTS property_tree)

add_library(retreat_core STATIC)
target_sources(retreat_core
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
            tstring.h
            debug.h
            scheduler/cron.h
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            algorithms.cppm
            controller.cppm
            settings.cppm
            settings_model.cppm
            system.cppm
            scheduler/CronEvent.cppm
            scheduler/Event.cppm
            scheduler/PeriodicEvent.cppm
            scheduler/Scheduler.cppm
            statemachine/Monitor.cppm
            statemachine/MonitorFactory.cppm
            statemachine/State.cppm
            statemachine/StateAlert.cppm
            statemachine/StateDelay.cppm
            statemachine/StateHalted.cppm
            statemachine/StateIdle.cppm
            statemachine/StateLocked.cppm
            statemachine/StateMachine.cppm
            statemachine/StateMachineImpl.cppm
            statemachine/StateMonitoring.cppm
            statemachine/StateRestore.cppm
            statemachine/StateSnapshot.cppm
            statemachine/StateSuspended.cppm
            statemachine/StateWindowAdapter.cppm
            statemachine/StateWindowFactory.cppm)

target_link_libraries(retreat_core
    PUBLIC Boost::property_tree
    PRIVATE retreat_options)

if(WIN32)
    # The Windows build uses wide strings throughout (see tstring.h).
    target_compile_definitions(retreat_core PUBLIC UNICODE _UNICODE)
endif()
