# -------------------------------------
# gpcc/src/time/os/linux_x64_tfc
# (Sources specific to operating system Linux on x64 architecture with TFC present)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/clock.cpp
              )
