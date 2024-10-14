# -------------------------------------
# gpcc/testcases/osal
# -------------------------------------
if((${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  add_subdirectory(${GPCC_OS})
endif()

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestAdvancedMutexLocker.cpp
               TestConditionVariable.cpp
               TestMutex.cpp
               TestMutexLocker.cpp
               TestPanic.cpp
               TestRWLock.cpp
               TestRWLockReadLocker.cpp
               TestRWLockWriteLocker.cpp
               TestSemaphore.cpp
               TestThread.cpp
               TestThreadRegistry.cpp)
