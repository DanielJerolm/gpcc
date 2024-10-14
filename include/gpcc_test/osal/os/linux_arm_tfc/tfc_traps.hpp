/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef TFC_TRAPS_HPP_202409302105
#define TFC_TRAPS_HPP_202409302105

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detection of threads that want to block on an OSAL primitive with timeout already expired.
 *
 * Some unit test cases might treat this as an error and can use this class to add a failure to the test case if a
 * thread attempts to block with an already expired timeout.
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
 * Note:
 * - There must be no more than one instance with monitoring enabled in the process at any time.
 * - Monitoring ends by calling @ref EndMonitoring() or when the instance is destroyed.
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
    void EndMonitoring(void);

  private:
    /// Enabled-state of the trap.
    bool enabled;
};

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detection of potential unreproducible behaviour in unit test cases.
 *
 * # Purpose
 * Some unit test cases may require a strictly reproducible order of thread execution. TFC guarantees reproducible
 * results, if no more than one thread's sleep or timeout ends at the same point in time on the emulated system clock.
 * [Details...](@ref GPCC_TIME_FLOW_CONTROL_REPRODUCIBILITY)
 *
 * The trap implemented by this class will trigger, if a thread is blocked (either by timeout or by sleep), and if there
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
 * Note:
 * - There must be no more than one instance with monitoring enabled in the process at any time.
 * - Monitoring ends by calling @ref EndMonitoring() or when the instance is destroyed.
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
    void EndMonitoring(void);

  private:
    /// Enabled-state of the trap.
    bool enabled;
};

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Detection of unreproducible behaviour in unit test cases.
 *
 * # Purpose
 * Some unit test cases may require a strictly reproducible order of thread execution. TFC guarantees reproducible
 * results, if no more than one thread's sleep or timeout ends at the same point in time on the emulated system clock.
 * [Details...](@ref GPCC_TIME_FLOW_CONTROL_REPRODUCIBILITY)
 *
 * The trap implemented by this class will trigger, if TFC increments the emulated system time and more than one thread
 * is switched into a runnable state. This class therefore triggers when unreproducible behaviour __actually__
 * __happens__. This class can be used in conjunction with class @ref PotentialUnreproducibleBehaviourTrap, which will
 * trap when more than one thread is blocked until the same point in time, which is __before__ the unreproducible
 * behaviour occurs.
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
 * Note:
 * - There must be no more than one instance with monitoring enabled in the process at any time.
 * - Monitoring ends by calling @ref EndMonitoring() or when the instance is destroyed.
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
    void EndMonitoring(void);

  private:
    /// Enabled-state of the trap.
    bool enabled;
};

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #ifndef TFC_TRAPS_HPP_202409302105
#endif // #ifdef OS_LINUX_ARM_TFC
