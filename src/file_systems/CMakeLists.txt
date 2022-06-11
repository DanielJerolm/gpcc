# -------------------------------------
# gpcc/src/file_systems
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_LIST_DIR}/CLI/ISFCLICommands.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/CLI/ESSCLICommands.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/EEPROMSectionSystem.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/internal/BlockAccessor.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/internal/EEPROMSectionSystemInternals.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/internal/FreeBlockListBackup.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/internal/SectionReader.cpp
               ${CMAKE_CURRENT_LIST_DIR}/EEPROMSectionSystem/internal/SectionWriter.cpp
              )

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_LIST_DIR}/linux_fs/FileStorage.cpp
                 ${CMAKE_CURRENT_LIST_DIR}/linux_fs/internal/StdIOFileReader.cpp
                 ${CMAKE_CURRENT_LIST_DIR}/linux_fs/internal/StdIOFileWriter.cpp
                 ${CMAKE_CURRENT_LIST_DIR}/linux_fs/internal/tools.cpp
                 ${CMAKE_CURRENT_LIST_DIR}/linux_fs/internal/UnitTestDirProvider.cpp
                )
endif()
