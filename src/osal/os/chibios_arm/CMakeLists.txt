# -------------------------------------
# gpcc/src/osal/os/chibios_arm
# (Sources specific to operating system ChibiOS/RT on ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ConditionVariable.cpp
               Mutex.cpp
               Panic.cpp
               Semaphore.cpp
               Thread.cpp
              )
