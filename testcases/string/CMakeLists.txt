# -------------------------------------
# gpcc/testcases/string
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               Test_levenshtein_distance.cpp
               Test_tools.cpp
               TestSharedString.cpp
               TestStringComposer_bool.cpp
               TestStringComposer_char.cpp
               TestStringComposer_cstring.cpp
               TestStringComposer_double.cpp
               TestStringComposer_float.cpp
               TestStringComposer_int.cpp
               TestStringComposer_long_long.cpp
               TestStringComposer_long.cpp
               TestStringComposer_longdouble.cpp
               TestStringComposer_short.cpp
               TestStringComposer_stdstring.cpp
               TestStringComposer_unsigned_char.cpp
               TestStringComposer_unsigned_int.cpp
               TestStringComposer_unsigned_long_long.cpp
               TestStringComposer_unsigned_long.cpp
               TestStringComposer_unsigned_short.cpp
               TestStringComposer.cpp)
