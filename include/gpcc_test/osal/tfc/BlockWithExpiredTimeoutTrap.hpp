/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#ifndef BLOCKWITHEXPIREDTIMEOUTTRAP_HPP_202410292118
#define BLOCKWITHEXPIREDTIMEOUTTRAP_HPP_202410292118

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detects if a thread attempts to block on an OSAL primitive with an already expired timeout value and adds a
 *        failure to the current unittest case.
 *
 * The actual trap is implemented in class @ref gpcc::osal::internal::TFCCore. This class provides the user API for the
 * trap.
 *
 * # Operation
 * - Monitoring can be enabled and disabled via @ref BeginMonitoring() and @ref EndMonitoring().
 * - If a thread attempts to block with timeout already expired while monitoring is enabled, then:
 *   - Class [TFCCore](@ref gpcc::osal::internal::TFCCore) will print a message to `std::cout`.
 *   - The trap's trigger state will be set to _triggered_.
 * - @ref EndMonitoring() will add a failure to the current unittest case, if the trap has been triggered.
 * - The trap's trigger state can be queried and reset while monitoring is enabled.
 *
 * Note:
 * - There may be multiple instances of this class at the same time (though this is unusual), but no more than one of
 *   them is allowed to have monitoring enabled.
 * - Monitoring ends by calling @ref EndMonitoring() or when the instance is destroyed.\n
 *   Note that only @ref EndMonitoring() will add a failure to the current test case.
 *
 * # Usage
 * In the following example, any attempt to block on an OSAL primitive with timeout already expired shall be detected:
 * ~~~{.cpp}
 * class MyTestFixture : public testing::Test
 * {
 *   public:
 *     MyTestFixture(void);
 *
 *   protected:
 *     ~MyTestFixture(void);
 *
 *     void SetUp(void) override;
 *     void TearDown(void) override;
 *
 *   private:
 *     gpcc_tests::osal::tfc::BlockWithExpiredTimeoutTrap trap;
 * };
 *
 * MyTestFixture::MyTestFixture(void)
 * : testing::Test()
 * , trap()
 * {
 * }
 *
 * MyTestFixture::~MyTestFixture(void)
 * {
 * }
 *
 * void MyTestFixture::SetUp(void)
 * {
 *   trap.BeginMonitoring();
 * }
 *
 * void MyTestFixture::TearDown(void)
 * {
 *   trap.EndMonitoring();
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class BlockWithExpiredTimeoutTrap final
{
  public:
    BlockWithExpiredTimeoutTrap(void) noexcept;
    BlockWithExpiredTimeoutTrap(BlockWithExpiredTimeoutTrap const &) = delete;
    BlockWithExpiredTimeoutTrap(BlockWithExpiredTimeoutTrap &&) = delete;
    ~BlockWithExpiredTimeoutTrap(void);

    BlockWithExpiredTimeoutTrap& operator=(BlockWithExpiredTimeoutTrap const &) = delete;
    BlockWithExpiredTimeoutTrap& operator=(BlockWithExpiredTimeoutTrap &&) = delete;

    void BeginMonitoring(void);
    bool QueryAndReset(void);
    void EndMonitoring(void);

  private:
    /// Enabled-state of the trap.
    bool enabled;
};

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // BLOCKWITHEXPIREDTIMEOUTTRAP_HPP_202410292118
#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
