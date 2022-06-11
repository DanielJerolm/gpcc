# -------------------------------------
# gpcc/src/Compiler/compiler/gcc_arm
# (Sources specific to gcc-compiler building for ARM architecture)
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/builtins.cpp
              )
