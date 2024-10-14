# -------------------------------------
# gpcc/testcases/execution
# -------------------------------------
add_subdirectory(async)
add_subdirectory(cyclic)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestUnittestDurationLimiter.cpp)
