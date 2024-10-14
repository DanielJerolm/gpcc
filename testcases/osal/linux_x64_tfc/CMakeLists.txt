# -------------------------------------
# gpcc/testcases/osal/linux_x64_tfc
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestAdvancedUnmanagedMutexLocker.cpp
               TestTFCCore.cpp
               TestThreadBlocker.cpp
               TestTimeLimitedThreadBlocker.cpp
               TestTraps.cpp
               TestUnmanagedConditionVariable.cpp
               TestUnmanagedMutex.cpp
               TestUnmanagedMutexLocker.cpp)
