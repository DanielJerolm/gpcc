/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#ifndef IOBJECTNOTIFIABLEMOCK_HPP_201809042025
#define IOBJECTNOTIFIABLEMOCK_HPP_201809042025

#include <gpcc/cood/Object.hpp>
#include <gpcc/cood/IObjectNotifiable.hpp>
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
