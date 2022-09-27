# -------------------------------------
# gpcc/src/osal/os/linux_arm_tfc
# (Sources specific to operating system Linux on ARM architecture with TFC present)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
               internal/AdvancedUnmanagedMutexLocker.cpp
               internal/TFCCore.cpp
               internal/ThreadBlocker.cpp
               internal/TimeLimitedThreadBlocker.cpp
               internal/UnmanagedConditionVariable.cpp
               internal/UnmanagedMutex.cpp
              )
