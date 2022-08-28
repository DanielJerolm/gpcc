/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "IStreamReader.hpp"
#include <gpcc/compiler/definitions.hpp>

namespace gpcc   {
namespace Stream {

#ifndef __DOXYGEN__

#if GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
IStreamReader::Endian const IStreamReader::nativeEndian = IStreamReader::Endian::Little;
#elif GPCC_SYSTEMS_ENDIAN == GPCC_BIG
IStreamReader::Endian const IStreamReader::nativeEndian = IStreamReader::Endian::Big;
#else
#error "Endian not supported"
#endif

#endif

}
}
