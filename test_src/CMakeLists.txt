# -------------------------------------
# gpcc/test_src
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               cli/FakeTerminal.cpp
               cood/remote_access/roda_itf/RODAN_Listener.cpp
               execution/UnittestDurationLimiter.cpp
               log/backends/Backend_Recorder.cpp)
