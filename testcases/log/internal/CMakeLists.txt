# -------------------------------------
# gpcc/testcases/log/internal
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestCStringLogMessage.cpp
               TestCStringLogMessageTS.cpp
               TestRomConstExceptionLogMessage.cpp
               TestRomConstExceptionLogMessageTS.cpp
               TestRomConstLogMessage.cpp
               TestRomConstLogMessageTS.cpp
               TestStringExceptionLogMessage.cpp
               TestStringExceptionLogMessageTS.cpp
               TestStringLogMessage.cpp
               TestStringLogMessageTS.cpp)
