# -------------------------------------
# gpcc/testcases/execution/async
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestDeferredWorkPackage.cpp
               TestDeferredWorkQueue.cpp
               TestDWQwithThread.cpp
               TestSuspendableDWQwithThread.cpp
               TestWorkPackage.cpp
               TestWorkQueue.cpp)
