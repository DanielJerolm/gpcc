# -------------------------------------
# gpcc/testcases/cli
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               internal/TestReturnKeyFilter.cpp
               internal/TestTerminalRxParser.cpp
               TestCLI_withICLINotifiable.cpp
               TestCLI.cpp
               TestCommand.cpp
               TestFakeTerminal.cpp)
