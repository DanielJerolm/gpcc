# -------------------------------------
# gpcc/src/string
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               levenshtein_distance.cpp
               SharedString.cpp
               StringComposer.cpp
               tools.cpp
              )
