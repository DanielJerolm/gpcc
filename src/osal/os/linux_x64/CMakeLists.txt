# -------------------------------------
# gpcc/src/osal/os/linux_x64
# (Sources specific to operating system Linux on x64 architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
              )
