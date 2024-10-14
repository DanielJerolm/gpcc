# -------------------------------------
# gpcc/testcases/cood/cli
# -------------------------------------
add_subdirectory(internal)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_string_conversion.cpp
               TestCLIAdapterForCANOpen.cpp)
