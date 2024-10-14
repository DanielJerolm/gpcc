# -------------------------------------
# gpcc/testcases/file_systems/eeprom_section_system
# -------------------------------------
add_subdirectory(cli)
add_subdirectory(internal)

target_sources(${PROJECT_NAME}_testcases
               PRIVATE
               EEPROMSectionSystemTestFixture.cpp
               FakeEEPROM.cpp
               FakeEEPROMUndo.cpp
               RandomData.cpp
               TestEEPROMSectionSystem.cpp
               TestFakeEEPROM.cpp)
