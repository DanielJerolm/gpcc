# -------------------------------------
# gpcc/src/time
# -------------------------------------
add_subdirectory(os/${GPCC_OS})

target_sources(${PROJECT_NAME}
               PRIVATE
               TimePoint.cpp
               TimeSpan.cpp
              )
