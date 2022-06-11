# -------------------------------------
# gpcc/src/log
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/backends/Backend_CLI.cpp
               ${CMAKE_CURRENT_LIST_DIR}/backends/Backend_CLILogHistory.cpp
               ${CMAKE_CURRENT_LIST_DIR}/backends/Backend.cpp
               ${CMAKE_CURRENT_LIST_DIR}/cli/commands.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/CStringLogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/CStringLogMessageTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/LogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/RomConstExceptionLogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/RomConstExceptionLogMessageTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/RomConstLogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/RomConstLogMessageTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/StringExceptionLogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/StringExceptionLogMessageTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/StringLogMessage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/StringLogMessageTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/logfacilities/ThreadedLogFacility.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
               ${CMAKE_CURRENT_LIST_DIR}/log_levels.cpp
               ${CMAKE_CURRENT_LIST_DIR}/log_tools.cpp
              )
