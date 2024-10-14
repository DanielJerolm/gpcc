# -------------------------------------
# gpcc/testcases/stream
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestIStreamReader.cpp
               TestIStreamWriter.cpp
               TestMemStreamReader.cpp
               TestMemStreamWriter.cpp)
