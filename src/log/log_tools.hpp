/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifndef LOG_TOOLS_HPP_201612150930
#define LOG_TOOLS_HPP_201612150930

#include "logfacilities/ILogFacilityCtrl.hpp"
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
