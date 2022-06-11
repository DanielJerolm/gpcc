/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2020 Daniel Jerolm

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

#if defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC)

#include "gpcc/src/hash/md5.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <stdexcept>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

namespace gpcc_tests {
namespace hash       {

using gpcc::hash::MD5Sum;

// This test implements a benchmark for MD5Sum(...). MD5Sum() will process a file. The file must be specified manually
// (pFileName). Googletest will measure execution time. Run this multiple times to allow the system to cache the
// file in RAM.
#if 0
TEST(gpcc_hash_md5_Tests, MD5Sum_Benchmark)
{
  char const * const pFileName = "/home/user/somefile.bin";

  int const fd = open(pFileName, O_RDONLY);
  if (fd == -1)
    throw std::runtime_error("MD5Sum_Benchmark: Could not open file");

  ON_SCOPE_EXIT(closeFile) { close(fd); };

  struct stat s;
  if (fstat(fd, &s) == -1)
    throw std::runtime_error("MD5Sum_Benchmark: 'fstat' failed");

  if (!S_ISREG(s.st_mode))
    throw std::logic_error("MD5Sum_Benchmark: The given file is not a regular file");

  void* p = mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (p == MAP_FAILED)
    throw std::runtime_error("MD5Sum_Benchmark: Could not map the file");

  ON_SCOPE_EXIT(unmapFile) { munmap(p, s.st_size); };

  auto md5 = MD5Sum(p, s.st_size);

  std::string md5str;
  for (auto const u8 : md5)
    md5str += gpcc::string::ToHexNoPrefix(u8, 2U);

  std::cout << "MD5: " << md5str << std::endl;
}
#endif

}
}

#endif // defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC)
