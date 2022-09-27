/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
