# -------------------------------------
# gpcc/testcases/file_systems/linux_fs
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_tools.cpp
               TestFileStorage.cpp
               TestUnitTestDirProvider.cpp)
