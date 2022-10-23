# -------------------------------------
# gpcc/src/cood
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               cli/CLIAdapterBase.cpp
               cli/CLIAdapterForCANOpen.cpp
               cli/internal/CAReadArgsParser.cpp
               cli/internal/CAWriteArgsParser.cpp
               cli/internal/EnumerateArgsParser.cpp
               cli/internal/InfoArgsParser.cpp
               cli/internal/ReadArgsParser.cpp
               cli/internal/WriteArgsParser.cpp
               cli/string_conversion.cpp
               data_types.cpp
               ObjectARRAY.cpp
               ObjectARRAY_wicb.cpp
               Object.cpp
               ObjectDictionary.cpp
               ObjectPtr.cpp
               ObjectRECORD.cpp
               ObjectRECORD_wicb.cpp
               ObjectVAR.cpp
               ObjectVAR_wicb.cpp
               remote_access/infrastructure/Multiplexer.cpp
               remote_access/infrastructure/MultiplexerPort.cpp
               remote_access/infrastructure/MultiRODACLIClientBase.cpp
               remote_access/infrastructure/MultiRODACLIClient.cpp
               remote_access/infrastructure/RemoteAccessServer.cpp
               remote_access/infrastructure/RODACLIClientBase.cpp
               remote_access/infrastructure/SingleRODACLIClientBase.cpp
               remote_access/infrastructure/SingleRODACLIClient.cpp
               remote_access/infrastructure/ThreadBasedRemoteAccessServer.cpp
               remote_access/infrastructure/WorkQueueBasedRemoteAccessServer.cpp
               remote_access/requests_and_responses/ObjectEnumRequest.cpp
               remote_access/requests_and_responses/ObjectEnumResponse.cpp
               remote_access/requests_and_responses/ObjectInfoRequest.cpp
               remote_access/requests_and_responses/ObjectInfoResponse.cpp
               remote_access/requests_and_responses/PingRequest.cpp
               remote_access/requests_and_responses/PingResponse.cpp
               remote_access/requests_and_responses/ReadRequest.cpp
               remote_access/requests_and_responses/ReadRequestResponse.cpp
               remote_access/requests_and_responses/RequestBase.cpp
               remote_access/requests_and_responses/ResponseBase.cpp
               remote_access/requests_and_responses/ReturnStackItem.cpp
               remote_access/requests_and_responses/WriteRequest.cpp
               remote_access/requests_and_responses/WriteRequestResponse.cpp
               sdo_abort_codes.cpp
              )

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 remote_access/infrastructure/IPCClient.cpp
                 remote_access/infrastructure/IPCServer.cpp
                )
endif()
