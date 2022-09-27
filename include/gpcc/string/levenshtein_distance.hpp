/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef LEVENSHTEIN_DISTANCE_HPP_201701151658
#define LEVENSHTEIN_DISTANCE_HPP_201701151658

#include <string>
#include <cstddef>

namespace gpcc {
namespace string {

size_t LevenshteinDistance(std::string const & str1, std::string const & str2, bool const caseSensitive);
size_t LevenshteinDistance(std::string const & str1, char const * const pStr2, bool const caseSensitive);
size_t LevenshteinDistance(char const * const pStr1, char const * const pStr2, bool const caseSensitive);

} // namespace string
} // namespace gpcc

#endif // LEVENSHTEIN_DISTANCE_HPP_201701151658
