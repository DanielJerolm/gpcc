# -------------------------------------
# gpcc/src/osal
# -------------------------------------
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/os/${GPCC_OS})

target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/universal/AdvancedMutexLocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/universal/RWLock.cpp
               ${CMAKE_CURRENT_LIST_DIR}/universal/RWLockReadLocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/universal/RWLockWriteLocker.cpp
               ${CMAKE_CURRENT_LIST_DIR}/universal/ThreadRegistry.cpp
              )
