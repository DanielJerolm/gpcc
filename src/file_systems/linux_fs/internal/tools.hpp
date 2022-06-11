/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#ifndef TOOLS_HPP_201805052150
#define TOOLS_HPP_201805052150

#include <list>
#include <string>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

void RemoveTrailingForwardSlash(std::string & s);
void EnsureTrailingForwardSlash(std::string & s);
void DeleteDirectoryContent(std::string dir);
void EnumerateFiles(std::string const & dir, std::list<std::string> & out, bool const recursive);
bool CheckFileOrDirExists(std::string const & name);
bool CheckFileExists(std::string const & name);
bool CheckDirExists(std::string const & name);
bool CheckFileName(std::string const & name, bool const acceptPath, bool const checkFileOnly);
bool CheckDirectoryName(std::string const & name, bool const checkDirectoryOnly);
bool CheckNotTopDir(std::string const & path) noexcept;

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // TOOLS_HPP_201805052150
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
