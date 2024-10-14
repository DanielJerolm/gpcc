# -------------------------------------
# gpcc/testcases/time
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_clock.cpp
               TestTimePoint.cpp
               TestTimeSpan.cpp)
