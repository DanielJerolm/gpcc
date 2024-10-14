# -------------------------------------
# gpcc/testcases/log/logfacilities
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               FakeBackend.cpp
               TestThreadedLogFacility.cpp)
