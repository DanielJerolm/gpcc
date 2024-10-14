# -------------------------------------
# gpcc/testcases/cood/remote_access/infrastructure
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestbenchMultiplexer.cpp
               TestbenchThreadBasedRAS.cpp
               TestbenchWorkQueueBasedRAS.cpp
               TestMultiplexer.cpp
               TestMultiplexerRODAItf.cpp
               TestMultiRODACLIClient.cpp
               TestSingleRODACLIClient.cpp
               TestThreadBasedRemoteAccessServer.cpp
               TestWorkQueueBasedRemoteAccessServer.cpp)
