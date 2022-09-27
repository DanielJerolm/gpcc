# -------------------------------------
# gpcc/src/osal/os/linux_x64_tfc
# (Sources specific to operating system Linux on x64 architecture with TFC present)
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
