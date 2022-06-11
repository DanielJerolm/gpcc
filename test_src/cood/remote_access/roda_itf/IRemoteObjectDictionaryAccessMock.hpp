/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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
