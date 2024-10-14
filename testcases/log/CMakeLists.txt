# -------------------------------------
# gpcc/testcases/log
# -------------------------------------
add_subdirectory(backends)
add_subdirectory(cli)
add_subdirectory(internal)
add_subdirectory(logfacilities)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_log_levels.cpp
               Test_log_tools.cpp
               TestLogger.cpp)
