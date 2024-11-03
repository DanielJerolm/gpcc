/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#ifndef UNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120
#define UNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detects unreproducible behaviour in unittest cases and adds a failure to the current unittest case.
 *
 * The actual trap is implemented in class @ref gpcc::osal::internal::TFCCore. This class provides the user API for the
 * trap.
 *
 * # Purpose
 * Some unit test cases may require a strictly reproducible order of thread execution. TFC guarantees reproducible
 * results, if no more than one thread's sleep or timeout ends at the same point in time on the emulated system clock.
 * [Details...](@ref GPCC_TIME_FLOW_CONTROL_REPRODUCIBILITY)
 *
 * The trap controlled by this class will trigger, if TFC increments the emulated system time and more than one thread
 * is switched into runnable state. This class therefore triggers when unreproducible behaviour __actually happens__.
 * This class can be used in conjunction with class @ref PotentialUnreproducibleBehaviourTrap, which will trap when more
 * than one thread is blocked until the same point in time, which is __before__ the unreproducible behaviour occurs.
 *
 * # Operation
 * - Monitoring can be enabled and disabled via @ref BeginMonitoring() and @ref EndMonitoring().
 * - If TFC increments the emulated system time and more than one thread is resumed because its sleep or timeout ends,
 *   then:
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
 * In the following example, potential unreproducible behaviour and actually happening unreproducible behaviour shall
 * be detected:
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
 *     gpcc_tests::osal::tfc::PotentialUnreproducibleBehaviourTrap trap1;
 *     gpcc_tests::osal::tfc::UnreproducibleBehaviourTrap trap2;
 * };
 *
 * MyTestFixture::MyTestFixture(void)
 * : testing::Test()
 * , trap1()
 * , trap2()
 * {
 * }
 *
 * MyTestFixture::~MyTestFixture(void)
 * {
 * }
 *
 * void MyTestFixture::SetUp(void)
 * {
 *   trap1.BeginMonitoring();
 *   trap2.BeginMonitoring();
 * }
 *
 * void MyTestFixture::TearDown(void)
 * {
 *   trap1.EndMonitoring();
 *   trap2.EndMonitoring();
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class UnreproducibleBehaviourTrap final
{
  public:
    UnreproducibleBehaviourTrap(void) noexcept;
    UnreproducibleBehaviourTrap(UnreproducibleBehaviourTrap const &) = delete;
    UnreproducibleBehaviourTrap(UnreproducibleBehaviourTrap &&) = delete;
    ~UnreproducibleBehaviourTrap(void);

    UnreproducibleBehaviourTrap& operator=(UnreproducibleBehaviourTrap const &) = delete;
    UnreproducibleBehaviourTrap& operator=(UnreproducibleBehaviourTrap &&) = delete;

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

#endif // UNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120
#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
