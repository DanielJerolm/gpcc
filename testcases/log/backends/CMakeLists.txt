# -------------------------------------
# gpcc/testcases/log/backends
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestBackend_CLI.cpp
               TestBackend_CLILogHistory.cpp
               TestBackend_Recorder.cpp)
