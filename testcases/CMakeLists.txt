# -------------------------------------
# gpcc/testcases
# -------------------------------------
if(GPCC_BuildEmptyTestCaseLibrary)
  target_sources(${PROJECT_NAME}_testcases
                 PRIVATE
                 empty.cpp)
else()
  target_sources(${PROJECT_NAME}_testcases
                 PRIVATE
                 callback/TestMultiCallback.cpp
                 callback/TestMultiCallback_OneParam.cpp
                 callback/TestMultiCallbackSM.cpp
                 callback/TestMultiCallbackSM_OneParam.cpp
                 cli/internal/TestReturnKeyFilter.cpp
                 cli/internal/TestTerminalRxParser.cpp
                 cli/TestCLI.cpp
                 cli/TestCLI_withICLINotifiable.cpp
                 cli/TestCommand.cpp
                 cli/TestFakeTerminal.cpp
                 compiler/TestCompilerBuiltins.cpp
                 compiler/TestCompilerDefs.cpp
                 container/TestBitField.cpp
                 container/TestIntrusiveDList.cpp
                 container/TestRAMBlock.cpp
                 cood/cli/internal/TestCAReadArgsParser.cpp
                 cood/cli/internal/TestCAWriteArgsParser.cpp
                 cood/cli/internal/TestEnumerateArgsParser.cpp
                 cood/cli/internal/TestInfoArgsParser.cpp
                 cood/cli/internal/TestReadArgsParser.cpp
                 cood/cli/internal/TestWriteArgsParser.cpp
                 cood/cli/TestCLIAdapterForCANOpen.cpp
                 cood/cli/Test_string_conversion.cpp
                 cood/ObjectVAR_wicb_withASM.cpp
                 cood/ObjectVARwithASM.cpp
                 cood/remote_access/infrastructure/TestbenchMultiplexer.cpp
                 cood/remote_access/infrastructure/TestbenchThreadBasedRAS.cpp
                 cood/remote_access/infrastructure/TestbenchWorkQueueBasedRAS.cpp
                 cood/remote_access/infrastructure/TestMultiplexer.cpp
                 cood/remote_access/infrastructure/TestMultiplexerRODAItf.cpp
                 cood/remote_access/infrastructure/TestMultiRODACLIClient.cpp
                 cood/remote_access/infrastructure/TestSingleRODACLIClient.cpp
                 cood/remote_access/infrastructure/TestThreadBasedRemoteAccessServer.cpp
                 cood/remote_access/infrastructure/TestWorkQueueBasedRemoteAccessServer.cpp
                 cood/remote_access/requests_and_responses/TestObjectEnumRequest.cpp
                 cood/remote_access/requests_and_responses/TestObjectEnumResponse.cpp
                 cood/remote_access/requests_and_responses/TestObjectInfoRequest.cpp
                 cood/remote_access/requests_and_responses/TestObjectInfoResponse.cpp
                 cood/remote_access/requests_and_responses/TestPingRequest.cpp
                 cood/remote_access/requests_and_responses/TestPingResponse.cpp
                 cood/remote_access/requests_and_responses/TestReadRequest.cpp
                 cood/remote_access/requests_and_responses/TestReadRequestResponse.cpp
                 cood/remote_access/requests_and_responses/TestRequestBase.cpp
                 cood/remote_access/requests_and_responses/TestResponseBase.cpp
                 cood/remote_access/requests_and_responses/TestReturnStackItem.cpp
                 cood/remote_access/requests_and_responses/TestWriteRequest.cpp
                 cood/remote_access/requests_and_responses/TestWriteRequestResponse.cpp
                 cood/remote_access/roda_itf/TestbenchBase.cpp
                 cood/Test_data_types.cpp
                 cood/TestObjectARRAY_wicb.cpp
                 cood/TestObjectDictionary.cpp
                 cood/TestObjectPtr.cpp
                 cood/TestObjectRECORD_wicb.cpp
                 cood/TestObjectVAR_wicb.cpp
                 cood/Test_sdo_abort_codes.cpp
                 crc/Test_simple_crc.cpp
                 dsp/filter/TestIntegralFilter.cpp
                 dsp/filter/TestMedianFilter1D.cpp
                 execution/async/TestDeferredWorkPackage.cpp
                 execution/async/TestDeferredWorkQueue.cpp
                 execution/async/TestDWQwithThread.cpp
                 execution/async/TestSuspendableDWQwithThread.cpp
                 execution/async/TestWorkPackage.cpp
                 execution/async/TestWorkQueue.cpp
                 execution/cyclic/TestTriggeredThreadedCyclicExec.cpp
                 execution/cyclic/TestTTCEStartStopCtrl.cpp
                 execution/cyclic/Trace.cpp
                 execution/cyclic/TriggerProvider.cpp
                 execution/cyclic/UUT_TriggeredThreadedCyclicExec.cpp
                 execution/cyclic/UUT_TTCEStartStopCtrl.cpp
                 execution/cyclic/WaitUntilStoppedHelper.cpp
                 execution/TestUnittestDurationLimiter.cpp
                 file_systems/cli/TestIFSCLICommands.cpp
                 file_systems/eeprom_section_system/cli/TestESSCLICommands.cpp
                 file_systems/eeprom_section_system/EEPROMSectionSystemTestFixture.cpp
                 file_systems/eeprom_section_system/FakeEEPROM.cpp
                 file_systems/eeprom_section_system/FakeEEPROMUndo.cpp
                 file_systems/eeprom_section_system/internal/TestBlockAccessor.cpp
                 file_systems/eeprom_section_system/RandomData.cpp
                 file_systems/eeprom_section_system/TestEEPROMSectionSystem.cpp
                 file_systems/eeprom_section_system/TestFakeEEPROM.cpp
                 file_systems/linux_fs/TestFileStorage.cpp
                 file_systems/linux_fs/Test_tools.cpp
                 file_systems/linux_fs/TestUnitTestDirProvider.cpp
                 hash/TestMD5Benchmark.cpp
                 hash/TestMD5.cpp
                 log/backends/TestBackend_CLI.cpp
                 log/backends/TestBackend_CLILogHistory.cpp
                 log/backends/TestBackend_Recorder.cpp
                 log/cli/Test_commands.cpp
                 log/internal/TestCStringLogMessage.cpp
                 log/internal/TestCStringLogMessageTS.cpp
                 log/internal/TestRomConstExceptionLogMessage.cpp
                 log/internal/TestRomConstExceptionLogMessageTS.cpp
                 log/internal/TestRomConstLogMessage.cpp
                 log/internal/TestRomConstLogMessageTS.cpp
                 log/internal/TestStringExceptionLogMessage.cpp
                 log/internal/TestStringExceptionLogMessageTS.cpp
                 log/internal/TestStringLogMessage.cpp
                 log/internal/TestStringLogMessageTS.cpp
                 log/logfacilities/FakeBackend.cpp
                 log/logfacilities/TestThreadedLogFacility.cpp
                 log/TestLogger.cpp
                 log/Test_log_levels.cpp
                 log/Test_log_tools.cpp
                 math/Test_checks.cpp
                 math/Test_int_math.cpp
                 osal/linux_arm_tfc/TestAdvancedUnmanagedMutexLocker.cpp
                 osal/linux_arm_tfc/TestTFC.cpp
                 osal/linux_arm_tfc/TestThreadBlocker.cpp
                 osal/linux_arm_tfc/TestTimeLimitedThreadBlocker.cpp
                 osal/linux_arm_tfc/TestUnmanagedConditionVariable.cpp
                 osal/linux_arm_tfc/TestUnmanagedMutex.cpp
                 osal/linux_arm_tfc/TestUnmanagedMutexLocker.cpp
                 osal/linux_x64_tfc/TestAdvancedUnmanagedMutexLocker.cpp
                 osal/linux_x64_tfc/TestTFC.cpp
                 osal/linux_x64_tfc/TestThreadBlocker.cpp
                 osal/linux_x64_tfc/TestTimeLimitedThreadBlocker.cpp
                 osal/linux_x64_tfc/TestUnmanagedConditionVariable.cpp
                 osal/linux_x64_tfc/TestUnmanagedMutex.cpp
                 osal/linux_x64_tfc/TestUnmanagedMutexLocker.cpp
                 osal/TestAdvancedMutexLocker.cpp
                 osal/TestConditionVariable.cpp
                 osal/TestMutex.cpp
                 osal/TestMutexLocker.cpp
                 osal/TestPanic.cpp
                 osal/TestRWLock.cpp
                 osal/TestRWLockReadLocker.cpp
                 osal/TestRWLockWriteLocker.cpp
                 osal/TestSemaphore.cpp
                 osal/TestThread.cpp
                 osal/TestThreadRegistry.cpp
                 raii/Test_scope_guard.cpp
                 resource_management/memory/TestHeapManager.cpp
                 resource_management/memory/TestHeapManagerSPTS.cpp
                 resource_management/objects/internal/TestNamedRWLockEntry.cpp
                 resource_management/objects/TestHierarchicNamedRWLock.cpp
                 resource_management/objects/TestLargeDynamicNamedRWLock.cpp
                 resource_management/objects/TestSmallDynamicNamedRWLock.cpp
                 resource_management/semaphores/TestNonBlockingSemaphore.cpp
                 stdif/i2c/Test_tools.cpp
                 stdif/storage/TestIRandomAccessStorageCLI.cpp
                 stream/TestIStreamReader.cpp
                 stream/TestIStreamWriter.cpp
                 stream/TestMemStreamReader.cpp
                 stream/TestMemStreamWriter.cpp
                 string/Test_levenshtein_distance.cpp
                 string/TestSharedString.cpp
                 string/Test_tools.cpp
                 time/Test_clock.cpp
                 time/TestTimePoint.cpp
                 time/TestTimeSpan.cpp
                )
endif()
