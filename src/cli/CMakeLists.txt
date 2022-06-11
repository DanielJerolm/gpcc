# -------------------------------------
# gpcc/src/cli
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/CLI.cpp
               ${CMAKE_CURRENT_LIST_DIR}/Command.cpp
               ${CMAKE_CURRENT_LIST_DIR}/exceptions.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/ReturnKeyFilter.cpp
               ${CMAKE_CURRENT_LIST_DIR}/internal/TerminalRxParser.cpp
               ${CMAKE_CURRENT_LIST_DIR}/ISyncSerialIO_to_ITerminal.cpp
               ${CMAKE_CURRENT_LIST_DIR}/StdIO_to_ITerminal.cpp
              )
