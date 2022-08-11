# -------------------------------------
# gpcc/src/osal/os/linux_arm
# (Sources specific to operating system Linux on ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
              )
