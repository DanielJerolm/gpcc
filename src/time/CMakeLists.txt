# -------------------------------------
# gpcc/src/time
# -------------------------------------
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/os/${GPCC_OS})

target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/TimePoint.cpp
               ${CMAKE_CURRENT_LIST_DIR}/TimeSpan.cpp
              )
