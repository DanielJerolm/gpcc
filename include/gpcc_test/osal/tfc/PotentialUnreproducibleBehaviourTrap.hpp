/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#ifndef POTENTIALUNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120
#define POTENTIALUNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detects potential unreproducible behaviour in unittest cases and adds a failure to the current unittest case.
 *
 * The actual trap is implemented in class @ref gpcc::osal::internal::TFCCore. This class provides the user API for the
 * trap.
 *
 * # Purpose
 * Some unit test cases may require a strictly reproducible order of thread execution. TFC guarantees reproducible
 * results, if no more than one thread's sleep or timeout ends at the same point in time on the emulated system clock.
 * [Details...](@ref GPCC_TIME_FLOW_CONTROL_REPRODUCIBILITY)
 *
 * The trap controlled by this class will trigger, if a thread is blocked (either by timeout or by sleep), and if there
 * is already another thread blocked until the __exactly same point in time__. This trap therefore triggers if
 * unreproducible behaviour __could__ happen in the future:
 * - If the timeout expires and both threads are resumed at the same time, then the order in which the threads are
 *   scheduled is undefined and this results in an unreproducible sequence of actions in the unit test case.
 * - If any of the two threads is resumed before the timeout expires (e.g. by signalling a condition variable the thread
 *   is blocked on), then there will be no unreproducible behaviour.
 *
 * This class can be used in conjunction with class @ref UnreproducibleBehaviourTrap, which will trap if unreproducible
 * behaviour actually happens.
 *
 * # Operation
 * - Monitoring can be enabled and disabled via @ref BeginMonitoring() and @ref EndMonitoring().
 * - If a thread attempts to block with timeout and there is already another thread blocking with the same timeout,
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
class PotentialUnreproducibleBehaviourTrap final
{
  public:
    PotentialUnreproducibleBehaviourTrap(void) noexcept;
    PotentialUnreproducibleBehaviourTrap(PotentialUnreproducibleBehaviourTrap const &) = delete;
    PotentialUnreproducibleBehaviourTrap(PotentialUnreproducibleBehaviourTrap &&) = delete;
    ~PotentialUnreproducibleBehaviourTrap(void);

    PotentialUnreproducibleBehaviourTrap& operator=(PotentialUnreproducibleBehaviourTrap const &) = delete;
    PotentialUnreproducibleBehaviourTrap& operator=(PotentialUnreproducibleBehaviourTrap &&) = delete;

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

#endif // POTENTIALUNREPRODUCIBLEBEHAVIOURTRAP_HPP_202410292120
#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
