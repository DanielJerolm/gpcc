# -------------------------------------
# gpcc/src/string
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/levenshtein_distance.cpp
               ${CMAKE_CURRENT_LIST_DIR}/SharedString.cpp
               ${CMAKE_CURRENT_LIST_DIR}/tools.cpp
              )
