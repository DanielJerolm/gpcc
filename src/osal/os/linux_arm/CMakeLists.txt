# -------------------------------------
# gpcc/src/osal/os/linux_arm
# (Sources specific to operating system Linux on ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/ConditionVariable.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Mutex.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Panic.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Semaphore.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Thread.cpp
              )
