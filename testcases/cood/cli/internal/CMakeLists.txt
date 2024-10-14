# -------------------------------------
# gpcc/testcases/cood/cli/internal
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestCAReadArgsParser.cpp
               TestCAWriteArgsParser.cpp
               TestEnumerateArgsParser.cpp
               TestInfoArgsParser.cpp
               TestReadArgsParser.cpp
               TestWriteArgsParser.cpp)
