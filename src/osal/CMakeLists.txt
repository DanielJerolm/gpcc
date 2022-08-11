# -------------------------------------
# gpcc/src/osal
# -------------------------------------
add_subdirectory(os/${GPCC_OS})

target_sources(${PROJECT_NAME}
               PRIVATE
               universal/AdvancedMutexLocker.cpp
               universal/RWLock.cpp
               universal/RWLockReadLocker.cpp
               universal/RWLockWriteLocker.cpp
               universal/ThreadRegistry.cpp
              )
