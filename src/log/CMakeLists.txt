# -------------------------------------
# gpcc/src/log
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               backends/Backend_CLI.cpp
               backends/Backend_CLILogHistory.cpp
               backends/Backend.cpp
               cli/commands.cpp
               internal/CStringLogMessage.cpp
               internal/CStringLogMessageTS.cpp
               internal/LogMessage.cpp
               internal/RomConstExceptionLogMessage.cpp
               internal/RomConstExceptionLogMessageTS.cpp
               internal/RomConstLogMessage.cpp
               internal/RomConstLogMessageTS.cpp
               internal/StringExceptionLogMessage.cpp
               internal/StringExceptionLogMessageTS.cpp
               internal/StringLogMessage.cpp
               internal/StringLogMessageTS.cpp
               logfacilities/ThreadedLogFacility.cpp
               Logger.cpp
               log_levels.cpp
               log_tools.cpp
              )
