# -------------------------------------
# gpcc/src/time/os/linux_arm
# (Sources specific to operating system Linux on ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/clock.cpp
              )
