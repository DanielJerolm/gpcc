/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLEMOCK_HPP_202009250903
#define IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLEMOCK_HPP_202009250903

#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include "gmock/gmock.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

class IRemoteObjectDictionaryAccessNotifiableMock : public gpcc::cood::IRemoteObjectDictionaryAccessNotifiable
{
  public:
    MOCK_METHOD(void, OnReady, (size_t const maxRequestSize, size_t const maxResponseSize), (noexcept, override));
    MOCK_METHOD(void, OnDisconnected, (), (noexcept, override));
    MOCK_METHOD(void, OnRequestProcessed, (std::unique_ptr<gpcc::cood::ResponseBase> spResponse), (noexcept, override));

    MOCK_METHOD(void, LoanExecutionContext, (), (noexcept, override));
};

} // namespace gpcc_tests
} // namespace cood

#endif // IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLEMOCK_HPP_202009250903
