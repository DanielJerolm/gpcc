/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef LOG_TOOLS_HPP_201612150930
#define LOG_TOOLS_HPP_201612150930

#include <gpcc/log/logfacilities/ILogFacilityCtrl.hpp>
#include <stdexcept>

namespace gpcc {

namespace file_systems
{
  class IFileStorage;
}

namespace log {

/**
 * \ingroup GPCC_LOG
 * \brief Exception thrown if the version of a binary file containing tLogSrcConfig entries is not supported.
 */
class InvalidVersionError : public std::runtime_error
{
  public:
    inline InvalidVersionError(void) : std::runtime_error("File version not supported") {};
    virtual ~InvalidVersionError(void) noexcept = default;
};

void WriteLogSrcConfigToFile(std::vector<ILogFacilityCtrl::tLogSrcConfig> const & config,
                             file_systems::IFileStorage& fs,
                             std::string const & fileName);
std::vector<ILogFacilityCtrl::tLogSrcConfig> ReadLogSrcConfigFromFile(file_systems::IFileStorage& fs,
                                                                      std::string const & fileName);

void WriteLogSrcConfigToTextFile(std::vector<ILogFacilityCtrl::tLogSrcConfig> const & config,
                                 file_systems::IFileStorage& fs,
                                 std::string const & fileName,
                                 std::string const & headline = std::string());
std::vector<ILogFacilityCtrl::tLogSrcConfig> ReadLogSrcConfigFromTextFile(file_systems::IFileStorage& fs,
                                                                          std::string const & fileName);

} // namespace log
} // namespace gpcc

#endif // LOG_TOOLS_HPP_201612150930
