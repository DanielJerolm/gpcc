# -------------------------------------
# gpcc/src/cli
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               CLI.cpp
               Command.cpp
               exceptions.cpp
               internal/ReturnKeyFilter.cpp
               internal/TerminalRxParser.cpp
               ISyncSerialIO_to_ITerminal.cpp
               StdIO_to_ITerminal.cpp
              )
