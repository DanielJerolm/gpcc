/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2021 Daniel Jerolm

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

#ifndef IOBJECTNOTIFIABLEMOCK_HPP_201809042025
#define IOBJECTNOTIFIABLEMOCK_HPP_201809042025

#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/IObjectNotifiable.hpp"
#include "gmock/gmock.h"

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;

using namespace testing;

// Mock that can be used as a sink for callbacks emitted by class cood::Object (or any of its subclasses) in unit tests.
class IObjectNotifiableMock : public gpcc::cood::IObjectNotifiable
{
  public:
    MOCK_METHOD(SDOAbortCode, OnBeforeRead, (Object const * pObj, uint8_t subindex, bool ca, bool querySize), (override));
    MOCK_METHOD(SDOAbortCode, OnBeforeWrite, (Object const * pObj, uint8_t subindex, bool ca, uint8_t SI0, void const * pData), (override));
    MOCK_METHOD(void, OnAfterWrite, (Object const * pObj, uint8_t subindex, bool ca), (override));
};

} // namespace gpcc_tests
} // namespace cood

#endif // IOBJECTNOTIFIABLEMOCK_HPP_201809042025
