# -------------------------------------
# gpcc/testcases/cood/remote_access/requests_and_responses
# -------------------------------------
target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               TestObjectEnumRequest.cpp
               TestObjectEnumResponse.cpp
               TestObjectInfoRequest.cpp
               TestObjectInfoResponse.cpp
               TestPingRequest.cpp
               TestPingResponse.cpp
               TestReadRequest.cpp
               TestReadRequestResponse.cpp
               TestRequestBase.cpp
               TestResponseBase.cpp
               TestReturnStackItem.cpp
               TestWriteRequest.cpp
               TestWriteRequestResponse.cpp)
