# -------------------------------------
# gpcc/src/Stream
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/IStreamReader.cpp
               ${CMAKE_CURRENT_LIST_DIR}/IStreamWriter.cpp
               ${CMAKE_CURRENT_LIST_DIR}/MemStreamReader.cpp
               ${CMAKE_CURRENT_LIST_DIR}/MemStreamWriter.cpp
               ${CMAKE_CURRENT_LIST_DIR}/StreamReaderBase.cpp
               ${CMAKE_CURRENT_LIST_DIR}/StreamWriterBase.cpp
              )
