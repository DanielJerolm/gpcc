# -------------------------------------
# gpcc/testcases/execution/cyclic
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestTriggeredThreadedCyclicExec.cpp
               TestTTCEStartStopCtrl.cpp
               Trace.cpp
               TriggerProvider.cpp
               UUT_TriggeredThreadedCyclicExec.cpp
               UUT_TTCEStartStopCtrl.cpp
               WaitUntilStoppedHelper.cpp)
