/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef IREMOTEOBJECTDICTIONARYACCESSMOCK_HPP_202106271051
#define IREMOTEOBJECTDICTIONARYACCESSMOCK_HPP_202106271051

#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/RequestBase.hpp"
#include "gmock/gmock.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

class IRemoteObjectDictionaryAccessMock : public gpcc::cood::IRemoteObjectDictionaryAccess
{
  public:
    MOCK_METHOD(void, Register, (gpcc::cood::IRemoteObjectDictionaryAccessNotifiable * const pNotifiable), (override));
    MOCK_METHOD(void, Unregister, (), (noexcept, override));
    MOCK_METHOD(void, Send, (std::unique_ptr<gpcc::cood::RequestBase> & spReq), (override));

    MOCK_METHOD(void, RequestExecutionContext, (), (override));
};

} // namespace gpcc_tests
} // namespace cood

#endif // IREMOTEOBJECTDICTIONARYACCESSMOCK_HPP_202106271051
