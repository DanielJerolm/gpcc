# -------------------------------------
# gpcc/testcases/resource_management/memory
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestHeapManager.cpp
               TestHeapManagerSPTS.cpp)
