# -------------------------------------
# gpcc/src/resource_management
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               memory/HeapManager.cpp
               memory/HeapManagerSPTS.cpp
               memory/HeapManagerStatistics.cpp
               memory/internal/FreeBlockPool.cpp
               memory/internal/MemoryDescriptorPool.cpp
               memory/MemoryDescriptor.cpp
               memory/MemoryDescriptorSPTS.cpp
               objects/HierarchicNamedRWLock.cpp
               objects/internal/HierarchicNamedRWLockNode.cpp
               objects/internal/NamedRWLockEntry.cpp
               objects/LargeDynamicNamedRWLock.cpp
               objects/SmallDynamicNamedRWLock.cpp
               semaphores/NonBlockingSemaphore.cpp
              )
