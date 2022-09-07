/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IRANDOMACCESSSTORAGECLI_HPP_202209072135
#define IRANDOMACCESSSTORAGECLI_HPP_202209072135

#include <string>

namespace gpcc  {
namespace cli   {
  class CLI;
}

namespace StdIf {
  class IRandomAccessStorage;
}}


namespace gpcc  {
namespace StdIf {

void CliCmdReadIRandomAccessStorage(std::string const & restOfLine,
                                    gpcc::cli::CLI & cli,
                                    IRandomAccessStorage* const pRAS);
void CliCmdWriteIRandomAccessStorage(std::string const & restOfLine,
                                     gpcc::cli::CLI & cli,
                                     IRandomAccessStorage* const pRAS);

} // namespace StdIf
} // namespace gpcc

#endif // IRANDOMACCESSSTORAGECLI_HPP_202209072135
