# -------------------------------------
# gpcc/testcases/callback
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestMultiCallback_OneParam.cpp
               TestMultiCallback.cpp
               TestMultiCallbackSM_OneParam.cpp
               TestMultiCallbackSM.cpp)
