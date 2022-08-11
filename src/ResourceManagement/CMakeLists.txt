# -------------------------------------
# gpcc/src/ResourceManagement
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               Memory/HeapManager.cpp
               Memory/HeapManagerSPTS.cpp
               Memory/HeapManagerStatistics.cpp
               Memory/internal/FreeBlockPool.cpp
               Memory/internal/MemoryDescriptorPool.cpp
               Memory/MemoryDescriptor.cpp
               Memory/MemoryDescriptorSPTS.cpp
               Objects/HierarchicNamedRWLock.cpp
               Objects/internal/HierarchicNamedRWLockNode.cpp
               Objects/internal/NamedRWLockEntry.cpp
               Objects/LargeDynamicNamedRWLock.cpp
               Objects/SmallDynamicNamedRWLock.cpp
               semaphores/NonBlockingSemaphore.cpp
              )
