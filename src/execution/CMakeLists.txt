# -------------------------------------
# gpcc/src/execution
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/async/DeferredWorkPackage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/async/DeferredWorkQueue.cpp
               ${CMAKE_CURRENT_LIST_DIR}/async/WorkPackage.cpp
               ${CMAKE_CURRENT_LIST_DIR}/async/WorkQueue.cpp
               ${CMAKE_CURRENT_LIST_DIR}/cyclic/TriggeredThreadedCyclicExec.cpp
               ${CMAKE_CURRENT_LIST_DIR}/cyclic/TTCEStartStopCtrl.cpp
              )
