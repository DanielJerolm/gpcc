# -------------------------------------
# gpcc/src/file_systems
# -------------------------------------
target_sources(${PROJECT_NAME}
               PRIVATE
               CLI/ISFCLICommands.cpp
               EEPROMSectionSystem/CLI/ESSCLICommands.cpp
               EEPROMSectionSystem/EEPROMSectionSystem.cpp
               EEPROMSectionSystem/internal/BlockAccessor.cpp
               EEPROMSectionSystem/internal/EEPROMSectionSystemInternals.cpp
               EEPROMSectionSystem/internal/FreeBlockListBackup.cpp
               EEPROMSectionSystem/internal/SectionReader.cpp
               EEPROMSectionSystem/internal/SectionWriter.cpp
              )

if((${GPCC_OS} STREQUAL "linux_arm") OR
   (${GPCC_OS} STREQUAL "linux_arm_tfc") OR
   (${GPCC_OS} STREQUAL "linux_x64") OR
   (${GPCC_OS} STREQUAL "linux_x64_tfc"))
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 linux_fs/FileStorage.cpp
                 linux_fs/internal/StdIOFileReader.cpp
                 linux_fs/internal/StdIOFileWriter.cpp
                 linux_fs/internal/tools.cpp
                 linux_fs/internal/UnitTestDirProvider.cpp
                )
endif()
