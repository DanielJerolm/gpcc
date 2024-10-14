# -------------------------------------
# gpcc/testcases/cood
# -------------------------------------
add_subdirectory(cli)
add_subdirectory(remote_access)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               ObjectVAR_wicb_withASM.cpp
               ObjectVARwithASM.cpp
               Test_data_types.cpp
               Test_sdo_abort_codes.cpp
               TestObjectARRAY_wicb.cpp
               TestObjectDictionary.cpp
               TestObjectPtr.cpp
               TestObjectRECORD_wicb.cpp
               TestObjectVAR_wicb.cpp)
