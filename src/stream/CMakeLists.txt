# -------------------------------------
# gpcc/src/stream
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               IStreamReader.cpp
               IStreamWriter.cpp
               MemStreamReader.cpp
               MemStreamWriter.cpp
               StreamReaderBase.cpp
               StreamWriterBase.cpp
              )
