/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

#if defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC)

#include <gpcc/hash/md5.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdexcept>
#include <string>
#include <iostream>

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
