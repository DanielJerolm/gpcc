# -------------------------------------
# gpcc/src/osal/os/linux_x64_tfc
# (Sources specific to operating system Linux on x64 architecture with TFC present)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/ConditionVariable.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Panic.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Semaphore.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Thread.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/AdvancedUnmanagedMutexLocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/TFCCore.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/ThreadBlocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/TimeLimitedThreadBlocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/UnmanagedConditionVariable.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/UnmanagedMutex.cpp
              )
