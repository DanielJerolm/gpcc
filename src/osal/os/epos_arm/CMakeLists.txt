# -------------------------------------
# gpcc/src/osal/os/epos_arm
# (Sources specific to operating system EPOS on ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
              )
