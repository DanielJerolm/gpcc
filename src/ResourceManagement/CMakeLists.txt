# -------------------------------------
# gpcc/src/ResourceManagement
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/Memory/HeapManager.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/HeapManagerSPTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/HeapManagerStatistics.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/internal/FreeBlockPool.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/internal/MemoryDescriptorPool.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/MemoryDescriptor.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Memory/MemoryDescriptorSPTS.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Objects/HierarchicNamedRWLock.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Objects/internal/HierarchicNamedRWLockNode.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Objects/internal/NamedRWLockEntry.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Objects/LargeDynamicNamedRWLock.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Objects/SmallDynamicNamedRWLock.cpp
               ${CMAKE_CURRENT_LIST_DIR}/semaphores/NonBlockingSemaphore.cpp
              )
