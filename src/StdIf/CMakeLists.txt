# -------------------------------------
# gpcc/src/StdIf
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/I2C/II2C_Tools.cpp
               ${CMAKE_CURRENT_LIST_DIR}/IRandomAccessStorageCLI.cpp
              )
