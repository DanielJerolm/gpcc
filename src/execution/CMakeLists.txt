# -------------------------------------
# gpcc/src/execution
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               async/DeferredWorkPackage.cpp
               async/DeferredWorkQueue.cpp
               async/DeferredWorkQueueWithThread.cpp
               async/WorkPackage.cpp
               async/WorkQueue.cpp
               cyclic/TriggeredThreadedCyclicExec.cpp
               cyclic/TTCEStartStopCtrl.cpp
              )
