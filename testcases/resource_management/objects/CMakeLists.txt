# -------------------------------------
# gpcc/testcases/resource_management/objects
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               internal/TestNamedRWLockEntry.cpp
               TestHierarchicNamedRWLock.cpp
               TestLargeDynamicNamedRWLock.cpp
               TestSmallDynamicNamedRWLock.cpp)
