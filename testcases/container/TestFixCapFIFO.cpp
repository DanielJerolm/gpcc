/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include <gpcc/container/FixCapFIFO.hpp>
#include <gpcc_test/compiler/warnings.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace container  {

using gpcc::container::FixCapFIFO;

/**
 * \brief Pushes @p count values onto the @p uut using `Push().`
 *
 * The UUT's status is queried via `IsFull()` before each invocation of `Push()`. This function expects, that the FIFO
 * is not full.
 *
 * The pushed values start with @p nextPushedValue. After each push the value is incremented, so that each value pushed
 * onto the UUT is unique.
 *
 * If no error (failed ASSERT_* or C++ exception) occurrs, then finally @p nextPushedValue will be updated, so that the
 * next call to this function seamlessly continues the number sequence of pushed values.
 *
 * \note  This must be invoked using `ASSERT_NO_FATAL_FAILURE()`!
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @p uut is left in a valid, but undefined state.
 * - @p nextPushedValue is not modified.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam UUTTYPE
 * Type of @p uut, e.g. `FixCapFIFO<uint32_t, uint8_t>`.
 *
 * \tparam T
 * Type of @p nextPushedValue.
 *
 * - - -
 *
 * \param uut
 * Reference to unit under test.
 *
 * \param count
 * Number of elements to push onto @p uut.
 *
 * \param nextPushedValue
 * Next value to be pushed.\n
 * If this function succeeds, then the referenced value will be increased by @p count.
 */
template<typename UUTTYPE, typename T>
static void UsePush(UUTTYPE & uut,
                    size_t const count,
                    T & nextPushedValue)
{
  // !! This must be invoked using ASSERT_NO_FATAL_FAILURE() !!

  T value = nextPushedValue;
  size_t n = count;
  while (n != 0U)
  {
    ASSERT_FALSE(uut.IsFull()) << "UUT is full. n = " << n << ", next pushed value = " << value;
    ASSERT_TRUE(uut.Push(value));
    ++value;
    --n;
  }

  nextPushedValue = value;
}

/**
 * \brief Pops @p count values from @p uut using `Pop()` and checks the popped values.
 *
 * The UUT's status is queried via `IsEmpty()` before each invocation of `Pop()`. This function expects, that the FIFO
 * is not empty.
 *
 * The popped values are compared with an expected value. The first expected value is @p nextExpectedPoppedValue. The
 * expected value is incremented after each pop. This function is the counterpart of @ref UsePush().
 *
 * If no error (failed ASSERT_* or C++ exception) occurrs, then finally @p nextExpectedPoppedValue will be updated, so
 * that the next call to this function seamlessly continues the number sequence of expected values.
 *
 * \note  This must be invoked using `ASSERT_NO_FATAL_FAILURE()`!
 *
 * - - -
 *
 * \tparam UUTTYPE
 * Type of @p uut, e.g. `FixCapFIFO<uint32_t, uint8_t>`.
 *
 * \tparam T
 * Type of @p nextExpectedPoppedValue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @p uut is left in a valid, but undefined state.
 * - @p nextExpectedPoppedValue is not modified.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param uut
 * Reference to unit under test.
 *
 * \param count
 * Number of elements to be popped from @p uut.
 *
 * \param nextExpectedPoppedValue
 * Expectation for the next popped value.\n
 * If this function succeeds, then the referenced value will be increased by @p count.
 */
template<typename UUTTYPE, typename T>
static void UsePopAndCheckValues(UUTTYPE & uut,
                                 size_t const count,
                                 T & nextExpectedValue)
{
  // !! This must be invoked using ASSERT_NO_FATAL_FAILURE() !!

  T value = nextExpectedValue;
  size_t n = count;
  while (n != 0U)
  {
    ASSERT_FALSE(uut.IsEmpty()) << "UUT is empty. n = " << n << ", nextExpectedValue = " << value;
    T poppedValue = 0;
    ASSERT_TRUE(uut.Pop(poppedValue));
    ASSERT_EQ(poppedValue, value) << "Popped value is invalid. n = " << n;
    ++value;
    --n;
  }

  nextExpectedValue = value;
}

/**
 * \brief Pushes @p count values onto the @p uut using `PushMultiple().`
 *
 * The pushed values start with @p nextPushedValue. Each pushed value is incremented by one, so that each pushed value
 * is unique.
 *
 * If no error (failed ASSERT_* or C++ exception) occurrs, then finally @p nextPushedValue will be updated, so that the
 * next call to this function seamlessly continues the number sequence of pushed values.
 *
 * \note  This must be invoked using `ASSERT_NO_FATAL_FAILURE()`!
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @p uut is left in a valid, but undefined state.
 * - @p nextPushedValue is not modified.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam UUTTYPE
 * Type of @p uut, e.g. `FixCapFIFO<uint32_t, uint8_t>`.
 *
 * \tparam T
 * Type of @p nextPushedValue.
 *
 * - - -
 *
 * \param uut
 * Reference to unit under test.
 *
 * \param count
 * Number of elements to push onto @p uut.
 *
 * \param actuallyPushed
 * The number of pushed items is written into the referenced variable.\n
 * This may be less than @p count, if the UUT has not enough free slots.
 *
 * \param nextPushedValue
 * Next value to be pushed.\n
 * If this function succeeds, then the referenced value will be increased by @p actuallyPushed.
 */
template<typename UUTTYPE, typename T>
static void UsePushMultiple(UUTTYPE & uut,
                            size_t const count,
                            size_t & actuallyPushed,
                            T & nextPushedValue)
{
  // !! This must be invoked using ASSERT_NO_FATAL_FAILURE() !!

  // prepare vector with data to be pushed
  std::vector<T> data;
  data.reserve(count);
  for (size_t i = 0U; i < count; ++i)
    data.push_back(static_cast<T>(nextPushedValue + i));

  size_t const pushed = uut.PushMultiple(data.data(), data.size());
  ASSERT_LE(pushed, count);

  nextPushedValue += pushed;
  actuallyPushed = pushed;
}

/**
 * \brief Pops @p count values from @p uut using `PopMultiple()` and checks the popped values.
 *
 * The popped values are compared with an expected value. The first expected value is @p nextExpectedPoppedValue. The
 * expected value is incremented for each popped value. This function is the counterpart of @ref UsePushMultiple().
 *
 * If no error (failed ASSERT_* or C++ exception) occurrs, then finally @p nextExpectedPoppedValue will be updated, so
 * that the next call to this function seamlessly continues the number sequence of expected values.
 *
 * \note  This must be invoked using `ASSERT_NO_FATAL_FAILURE()`!
 *
 * - - -
 *
 * \tparam UUTTYPE
 * Type of @p uut, e.g. `FixCapFIFO<uint32_t, uint8_t>`.
 *
 * \tparam T
 * Type of @p nextExpectedPoppedValue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @p uut is left in a valid, but undefined state.
 * - @p nextExpectedPoppedValue is not modified.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param uut
 * Reference to unit under test.
 *
 * \param count
 * Number of elements to be popped from @p uut.
 *
 * \param actuallyPopped
 * The number of popped elements is written into the referenced variable.\n
 * This may be less than @p count, if the UUT does not contain enough elements.
 *
 * \param nextExpectedPoppedValue
 * Expectation for the next popped value.\n
 * If this function succeeds, then the referenced value will be increased by @p count.
 */
template<typename UUTTYPE, typename T>
static void UsePopMultipleAndCheckValues(UUTTYPE & uut,
                                         size_t const count,
                                         size_t & actuallyPopped,
                                         T & nextExpectedValue)
{
  // !! This must be invoked using ASSERT_NO_FATAL_FAILURE() !!

  // prepare vector for popped data
  std::vector<T> data(count);

  size_t const popped = uut.PopMultiple(data.data(), data.size());
  ASSERT_LE(popped, data.size());
  data.resize(popped);

  // check popped data
  T value = nextExpectedValue;
  for (auto const v: data)
  {
    ASSERT_EQ(v, value);
    ++value;
  }

  nextExpectedValue = value;
  actuallyPopped = popped;
}


TEST(gpcc_container_FixCapFIFO_Tests, Construction)
{
  std::array<size_t, 6U> const capacities{1U, 2U, 3U, 8U, 12U, 16U};

  for (auto const capacity: capacities)
  {
    FixCapFIFO<uint8_t, size_t> uut(capacity);

    EXPECT_EQ(uut.Capacity(), capacity);
    EXPECT_EQ(uut.Size(), 0U);
    EXPECT_TRUE(uut.IsEmpty());
    EXPECT_FALSE(uut.IsFull());
  }
}

TEST(gpcc_container_FixCapFIFO_Tests, Construction_InvalidArgs)
{
  using FixCapFIFOu8u8 = FixCapFIFO<uint8_t, uint8_t>;

  std::unique_ptr<FixCapFIFOu8u8> spUUT;

  // zero capacity is not allowed
  EXPECT_THROW(spUUT = std::make_unique<FixCapFIFOu8u8>(0U), std::invalid_argument);

  // capacity must not exceed maximum value of template parameter SIZET
  EXPECT_THROW(spUUT = std::make_unique<FixCapFIFOu8u8>(256U), std::invalid_argument);
}

TEST(gpcc_container_FixCapFIFO_Tests, Clear)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_TRUE(uut.Push(2U));

  uut.Clear();
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // check that push and pop work after clear
  ASSERT_TRUE(uut.Push(3U));
  uint32_t v = 0U;
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 3U);

  EXPECT_TRUE(uut.IsEmpty());
  EXPECT_FALSE(uut.IsFull());
  EXPECT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, ClearWhileEmpty)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);

  uut.Clear();
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // check that push and pop work after clear
  ASSERT_TRUE(uut.Push(3U));
  uint32_t v = 0U;
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 3U);

  EXPECT_TRUE(uut.IsEmpty());
  EXPECT_FALSE(uut.IsFull());
  EXPECT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, BasicPushPop_Capacity1)
{
  // This test case checks basic operation of the UUT's Push() and Pop() methods for an UUT with capacity 1.
  // gpcc_container_FixCapFIFO_Tests.PushPop performs a thorough test of these methods covering all possible
  // scenarios.

  FixCapFIFO<uint32_t, uint8_t> uut(1U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // attempt to pop empty FIFO
  uint32_t v = 0xDEADBEEFUL;
  ASSERT_FALSE(uut.Pop(v));
  EXPECT_EQ(v, 0xDEADBEEFUL);

  // Push "1"
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  // attempt to push to full FIFO
  ASSERT_FALSE(uut.Push(55U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  // Pop "1"
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // Push "2"
  ASSERT_TRUE(uut.Push(2U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  // Pop "2"
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 2U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, BasicPushPop_Capacity2)
{
  // This test case checks basic operation of the UUT's Push() and Pop() methods for an UUT with capacity 2.
  // gpcc_container_FixCapFIFO_Tests.PushPop performs a thorough test of these methods covering all possible
  // scenarios.

  FixCapFIFO<uint32_t, uint8_t> uut(2U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // attempt to pop empty FIFO
  uint32_t v = 0xDEADBEEFUL;
  ASSERT_FALSE(uut.Pop(v));
  EXPECT_EQ(v, 0xDEADBEEFUL);

  // Push "1"
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  // Pop "1"
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);

  // Push "2" and "3"
  ASSERT_TRUE(uut.Push(2U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  ASSERT_TRUE(uut.Push(3U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 2U);

  // attempt to push to full FIFO
  ASSERT_FALSE(uut.Push(55U));
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 2U);

  // Pop "2" and "3"
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 2U);
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 1U);

  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 3U);
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, BasicPushMultiplePopMultiple_Capacity1)
{
  // This test case checks basic operation of the UUT's PushMultiple() and PopMultiple() methods for an UUT with
  // capacity 1. gpcc_container_FixCapFIFO_Tests.PushMultiplePopMultiple performs a thorough test of these methods
  // covering all possible scenarios.

  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  FixCapFIFO<uint32_t, uint8_t> uut(1U);

  for (uint_fast8_t i = 0U; i < 2U; ++i)
  {
    // Attempt to push 2 values. 1 value should actually be pushed.
    size_t nbOfPushedValues = 0U;
    UsePushMultiple(uut, 2U, nbOfPushedValues, pushValue);
    ASSERT_EQ(nbOfPushedValues, 1U);

    ASSERT_FALSE(uut.IsEmpty());
    ASSERT_TRUE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 1U);

    // Attempt to push 2 value to a full FIFO.
    UsePushMultiple(uut, 2U, nbOfPushedValues, pushValue);
    ASSERT_EQ(nbOfPushedValues, 0U);

    ASSERT_FALSE(uut.IsEmpty());
    ASSERT_TRUE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 1U);

    // Attempt to pop 2 values. 1 value should actually be popped.
    size_t nbOfPoppedValues = 0U;
    UsePopMultipleAndCheckValues(uut, 2U, nbOfPoppedValues, popValue);
    ASSERT_EQ(nbOfPoppedValues, 1U);

    ASSERT_TRUE(uut.IsEmpty());
    ASSERT_FALSE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 0U);

    // Attempt to pop 2 values from an empty FIFO.
    nbOfPoppedValues = 0U;
    UsePopMultipleAndCheckValues(uut, 2U, nbOfPoppedValues, popValue);
    ASSERT_EQ(nbOfPoppedValues, 0U);

    ASSERT_TRUE(uut.IsEmpty());
    ASSERT_FALSE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 0U);
  }
}

TEST(gpcc_container_FixCapFIFO_Tests, BasicPushMultiplePopMultiple_Capacity2)
{
  // This test case checks basic operation of the UUT's PushMultiple() and PopMultiple() methods for an UUT with
  // capacity 2. gpcc_container_FixCapFIFO_Tests.PushMultiplePopMultiple performs a thorough test of these methods
  // covering all possible scenarios.

  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  FixCapFIFO<uint32_t, uint8_t> uut(2U);

  for (uint_fast8_t i = 0U; i < 2U; ++i)
  {
    // Attempt to push 3 values. 2 values should actually be pushed.
    size_t nbOfPushedValues = 0U;
    UsePushMultiple(uut, 3U, nbOfPushedValues, pushValue);
    ASSERT_EQ(nbOfPushedValues, 2U);

    ASSERT_FALSE(uut.IsEmpty());
    ASSERT_TRUE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 2U);

    // Attempt to push 2 value to a full FIFO.
    UsePushMultiple(uut, 2U, nbOfPushedValues, pushValue);
    ASSERT_EQ(nbOfPushedValues, 0U);

    ASSERT_FALSE(uut.IsEmpty());
    ASSERT_TRUE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 2U);

    // Attempt to pop 3 values. 2 values should actually be popped.
    size_t nbOfPoppedValues = 0U;
    UsePopMultipleAndCheckValues(uut, 3U, nbOfPoppedValues, popValue);
    ASSERT_EQ(nbOfPoppedValues, 2U);

    ASSERT_TRUE(uut.IsEmpty());
    ASSERT_FALSE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 0U);

    // Attempt to pop 2 values from an empty FIFO.
    nbOfPoppedValues = 0U;
    UsePopMultipleAndCheckValues(uut, 2U, nbOfPoppedValues, popValue);
    ASSERT_EQ(nbOfPoppedValues, 0U);

    ASSERT_TRUE(uut.IsEmpty());
    ASSERT_FALSE(uut.IsFull());
    ASSERT_EQ(uut.Size(), 0U);
  }
}

TEST(gpcc_container_FixCapFIFO_Tests, PushMultiple_Zero)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);

  // PushMultiple() with n = 0
  uint32_t const values[2] = {11U, 12U};
  ASSERT_EQ(uut.PushMultiple(values, 0U), 0U);

  // FIFO should still be empty
  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, PopMultiple_Zero)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_TRUE(uut.Push(2U));

  // PopMultiple() with n = 0
  uint32_t values[2] = { 0U, 0U };
  ASSERT_EQ(uut.PopMultiple(values, 0U), 0U);

  // values should not have changed
  EXPECT_EQ(values[0], 0U);
  EXPECT_EQ(values[1], 0U);

  // FIFO should still be full
  ASSERT_FALSE(uut.IsEmpty());
  ASSERT_TRUE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 2U);

  // Pop values
  uint32_t v = 0U;
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 2U);

  ASSERT_TRUE(uut.IsEmpty());
  ASSERT_FALSE(uut.IsFull());
  ASSERT_EQ(uut.Size(), 0U);
}

TEST(gpcc_container_FixCapFIFO_Tests, PushPop)
{
  // This test case pushes and pops different numbers of elements onto and from the UUT using Push() and Pop(),
  // starting at different states of the UUT. The UUT's state is comprised of the index of the read-pointer and the
  // write-pointer. All possible states are generated using nested loops.

  // counter for creating unique values pushed onto the UUT during this test and for checking the popped values
  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  size_t const capacity = 8U;

  // 1st variable of test: Initial read-index
  // Note: "<=" results in intentional wrap around of read-index
  for (size_t initialRdIdx = 0U; initialRdIdx <= capacity; ++initialRdIdx)
  {
    // 2nd varibale of test: Initial write-index
    // Note: "<=" results in intentional wrap around of write-index
    for (size_t initialWrIdx = 0U; initialWrIdx <= capacity; ++initialWrIdx)
    {
      // calculate the initial number of elements in the UUT after the precondition has been setup
      size_t const initialNbOfElements =   (initialRdIdx <= initialWrIdx) ?
                                           (initialWrIdx - initialRdIdx)
                                         : ((capacity + initialWrIdx) - initialRdIdx);

      // calculate the number of free slots in the UUT after the precondition has been setup
      size_t const freeSlots = capacity - initialNbOfElements;

      // 3rd variable of test: Number of elements pushed onto the FIFO after precondition has been established
      for (size_t n = 0U; n <= freeSlots; ++n)
      {
        if (popValue != pushValue)
          throw std::logic_error("Testcase internal error");

        FixCapFIFO<uint32_t, uint8_t> uut(capacity);

        // Setup test precondition:
        // Move read- and write-index into position using whitebox knowledge
        if (initialRdIdx <= initialWrIdx)
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
        }
        else
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, capacity, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
        }

        ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut.Size(), initialNbOfElements);

        // Preconditon established:
        // - read index is (initialRdIdx % capacity) now
        // - write index is (initialWrIdx % capacity) now

        // Test:
        // Push n values, check FIFO state, pop n values, check FIFO state
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, n, pushValue));

        size_t const expectedSize = initialNbOfElements + n;
        ASSERT_EQ(uut.IsEmpty(), (expectedSize == 0U));
        ASSERT_EQ(uut.IsFull(), (expectedSize == capacity));
        ASSERT_EQ(uut.Size(), expectedSize);

        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, n, popValue));

        ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut.Size(), initialNbOfElements);

        // Aftermath:
        // Pop and check 'initialNbOfElements' values due to the precondition
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialNbOfElements, popValue));

        ASSERT_TRUE(uut.IsEmpty());
        ASSERT_FALSE(uut.IsFull());
        ASSERT_EQ(uut.Size(), 0U);
      } // for n...
    } // for initialWrIdx...
  } // for initialRdIdx...
}

TEST(gpcc_container_FixCapFIFO_Tests, PushMultiplePopMultiple)
{
  // This test case pushes and pops different numbers of elements onto and from the UUT using PushMultiple() and
  // PopMultiple(), starting at different states of the UUT. The UUT state is comprised of the index of the
  // read-pointer and the write-pointer. All possible states are generated using nested loops.

  // counter for creating unique values pushed onto the UUT during this test and for checking the popped values
  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  size_t const capacity = 8U;

  // 1st variable of test: Initial read-index
  // Note: "<=" results in intentional wrap around of read-index
  for (size_t initialRdIdx = 0U; initialRdIdx <= capacity; ++initialRdIdx)
  {
    // 2nd varibale of test: Initial write-index
    // Note: "<=" results in intentional wrap around of write-index
    for (size_t initialWrIdx = 0U; initialWrIdx <= capacity; ++initialWrIdx)
    {
      // calculate the initial number of elements in the UUT after the precondition has been setup
      size_t const initialNbOfElements =   (initialRdIdx <= initialWrIdx) ?
                                           (initialWrIdx - initialRdIdx)
                                         : ((capacity + initialWrIdx) - initialRdIdx);

      // 3rd variable of test: Number of elements pushed onto the FIFO after precondition has been established
      // Note: n may exceed the number of free slots. This is by intention.
      for (size_t n = 0U; n <= capacity; ++n)
      {
        if (popValue != pushValue)
          throw std::logic_error("Testcase internal error");

        FixCapFIFO<uint32_t, uint8_t> uut(capacity);

        // Setup test precondition:
        // Move read- and write-index into position using whitebox knowledge
        if (initialRdIdx <= initialWrIdx)
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
        }
        else
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, capacity, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
        }

        ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut.Size(), initialNbOfElements);

        // Preconditon established:
        // - read index is (initialRdIdx % capacity) now
        // - write index is (initialWrIdx % capacity) now

        // Test:
        // Push n values, check FIFO state, pop n values, check FIFO state
        size_t nbOfPushedValues = 0U;
        UsePushMultiple(uut, n, nbOfPushedValues, pushValue);

        size_t const expectedSize = initialNbOfElements + nbOfPushedValues;
        ASSERT_EQ(uut.IsEmpty(), (expectedSize == 0U));
        ASSERT_EQ(uut.IsFull(), (expectedSize == capacity));
        ASSERT_EQ(uut.Size(), expectedSize);

        size_t nbOfPoppedValues = 0U;
        UsePopMultipleAndCheckValues(uut, n, nbOfPoppedValues, popValue);

        size_t const expectedNbOfElements = (initialNbOfElements + nbOfPushedValues) - nbOfPoppedValues;
        ASSERT_EQ(uut.IsEmpty(), (expectedNbOfElements == 0U));
        ASSERT_EQ(uut.IsFull(), (expectedNbOfElements == capacity));
        ASSERT_EQ(uut.Size(), expectedNbOfElements);

        // Aftermath:
        // Pop and check 'expectedNbOfElements' values due to the precondition
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, expectedNbOfElements, popValue));

        ASSERT_TRUE(uut.IsEmpty());
        ASSERT_FALSE(uut.IsFull());
        ASSERT_EQ(uut.Size(), 0U);
      } // for n...
    } // for initialWrIdx...
  } // for initialRdIdx...
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyCTOR)
{
  // This test generates all possible states of the UUT using nested loops and tests copy-construction of a new
  // instance from each state. The UUT's state is comprised of the index of the read-pointer and the write-pointer.
  // For each state, all possible numbers of elements are pushed/popped to/from the copied instance.

  // Counter for creating unique values pushed onto the original UUT during this test and for checking the popped
  // values. Note that the copied instances use their own number sequences.
  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  size_t const capacity = 8U;

  // 1st variable of test: Initial read-index
  // Note: "<=" results in intentional wrap around of read-index
  for (size_t initialRdIdx = 0U; initialRdIdx <= capacity; ++initialRdIdx)
  {
    // 2nd varibale of test: Initial write-index
    // Note: "<=" results in intentional wrap around of write-index
    for (size_t initialWrIdx = 0U; initialWrIdx <= capacity; ++initialWrIdx)
    {
      // calculate the initial number of elements in the UUT after the precondition has been setup
      size_t const initialNbOfElements =   (initialRdIdx <= initialWrIdx) ?
                                           (initialWrIdx - initialRdIdx)
                                         : ((capacity + initialWrIdx) - initialRdIdx);

      // calculate the number of free slots in the UUT after the precondition has been setup
      size_t const freeSlots = capacity - initialNbOfElements;

      if (popValue != pushValue)
        throw std::logic_error("Testcase internal error");

      FixCapFIFO<uint32_t, uint8_t> uut(capacity);

      // Setup test precondition:
      // Move read- and write-index into position using whitebox knowledge
      if (initialRdIdx <= initialWrIdx)
      {
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
      }
      else
      {
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, capacity, pushValue));
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
      }

      ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
      ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
      ASSERT_EQ(uut.Size(), initialNbOfElements);

      // Preconditon established:
      // - read index is (initialRdIdx % capacity) now
      // - write index is (initialWrIdx % capacity) now

      // 3rd variable of test: Create a copy of the UUT and push/pop 0..freeSlots elements to/from the copy
      for (size_t n = 0U; n <= freeSlots; ++n)
      {
        // create new instance using copy-construction
        FixCapFIFO<uint32_t, uint8_t> copyOfUUT(uut);

        // the copied instance uses its own sequence of values
        uint32_t copyOfPopValue = popValue;
        uint32_t copyOfPushValue = pushValue;

        // check status of copyOfUUT
        ASSERT_EQ(copyOfUUT.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(copyOfUUT.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(copyOfUUT.Size(), initialNbOfElements);
        ASSERT_EQ(copyOfUUT.Capacity(), capacity);

        if (n != 0U)
        {
          // push n values, check FIFO state, pop n values, check FIFO state
          ASSERT_NO_FATAL_FAILURE(UsePush(copyOfUUT, n, copyOfPushValue));

          size_t const expectedSize = initialNbOfElements + n;
          ASSERT_EQ(copyOfUUT.IsEmpty(), (expectedSize == 0U));
          ASSERT_EQ(copyOfUUT.IsFull(), (expectedSize == capacity));
          ASSERT_EQ(copyOfUUT.Size(), expectedSize);

          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(copyOfUUT, n, copyOfPopValue));

          ASSERT_EQ(copyOfUUT.IsEmpty(), (initialNbOfElements == 0U));
          ASSERT_EQ(copyOfUUT.IsFull(), (initialNbOfElements == capacity));
          ASSERT_EQ(copyOfUUT.Size(), initialNbOfElements);
        }

        // pop 'initialNbOfElements' elements from copyOfUUT that originate from the precondition
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(copyOfUUT, initialNbOfElements, copyOfPopValue));

        ASSERT_TRUE(copyOfUUT.IsEmpty());
        ASSERT_FALSE(copyOfUUT.IsFull());
        ASSERT_EQ(copyOfUUT.Size(), 0U);
      } // for n...

      // Finally check status and content of the original UUT.
      // It should not have been changed by creating all the copies.
      ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
      ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
      ASSERT_EQ(uut.Size(), initialNbOfElements);

      ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialNbOfElements, popValue));

      ASSERT_TRUE(uut.IsEmpty());
      ASSERT_FALSE(uut.IsFull());
      ASSERT_EQ(uut.Size(), 0U);
    } // for initialWrIdx...
  } // for initialRdIdx...
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignmentSelf)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_TRUE(uut.Push(3U));

  uut = uut;

  ASSERT_EQ(uut.Size(), 2U);
  uint32_t v = 0U;
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 3U);
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignment_OtherHasSmallerCapacity)
{
  FixCapFIFO<uint32_t, uint8_t> uut1(3U);
  ASSERT_TRUE(uut1.Push(1U));
  ASSERT_TRUE(uut1.Push(3U));

  FixCapFIFO<uint32_t, uint8_t> uut2(4U);
  ASSERT_TRUE(uut2.Push(55U));

  ASSERT_NO_THROW(uut2 = uut1);

  EXPECT_EQ(uut2.Capacity(), 3U);

  ASSERT_EQ(uut1.Size(), 2U);
  uint32_t v = 0U;
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 3U);

  ASSERT_EQ(uut2.Size(), 2U);
  v = 0U;
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 3U);
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignment_OtherHasLargerCapacity)
{
  FixCapFIFO<uint32_t, uint8_t> uut1(4U);
  ASSERT_TRUE(uut1.Push(1U));
  ASSERT_TRUE(uut1.Push(3U));

  FixCapFIFO<uint32_t, uint8_t> uut2(2U);
  ASSERT_TRUE(uut2.Push(55U));

  ASSERT_NO_THROW(uut2 = uut1);

  EXPECT_EQ(uut2.Capacity(), 4U);

  ASSERT_EQ(uut1.Size(), 2U);
  uint32_t v = 0U;
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 3U);

  ASSERT_EQ(uut2.Size(), 2U);
  v = 0U;
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 3U);
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignment_SameCapacity)
{
  FixCapFIFO<uint32_t, uint8_t> uut1(4U);
  ASSERT_TRUE(uut1.Push(1U));
  ASSERT_TRUE(uut1.Push(3U));
  ASSERT_TRUE(uut1.Push(8U));

  FixCapFIFO<uint32_t, uint8_t> uut2(4U);
  ASSERT_TRUE(uut2.Push(55U));

  uut2 = uut1;

  EXPECT_EQ(uut2.Capacity(), 4U);

  ASSERT_EQ(uut1.Size(), 3U);
  uint32_t v = 0U;
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 3U);
  ASSERT_TRUE(uut1.Pop(v));
  EXPECT_EQ(v, 8U);

  ASSERT_EQ(uut2.Size(), 3U);
  v = 0U;
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 3U);
  ASSERT_TRUE(uut2.Pop(v));
  EXPECT_EQ(v, 8U);
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignment_OtherIsEmpty)
{
  FixCapFIFO<uint32_t, uint8_t> uut1(4U);

  FixCapFIFO<uint32_t, uint8_t> uut2(2U);
  ASSERT_TRUE(uut2.Push(55U));

  uut2 = uut1;

  EXPECT_EQ(uut2.Capacity(), 4U);

  EXPECT_TRUE(uut1.IsEmpty());
  EXPECT_TRUE(uut2.IsEmpty());
}

TEST(gpcc_container_FixCapFIFO_Tests, CopyAssignment)
{
  // This test generates all possible states of the UUT using nested loops and tests copy-assignment of the UUT's
  // content to a 2nd instance from each state. The UUT's state is comprised of the index of the read-pointer and the
  // write-pointer. For each state, all possible numbers of elements are pushed/popped to/from the 2nd instance that was
  // the target of the copy-assignment.

  // Counter for creating unique values pushed onto the original UUT during this test and for checking the popped
  // values. Note that the 2nd instance uses its own number sequence.
  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  size_t const capacity = 8U;

  // 2nd instance (target of copy-assignment through the whole test)
  FixCapFIFO<uint32_t, uint8_t> uut2(capacity);

  // 1st variable of test: Initial read-index
  // Note: "<=" results in intentional wrap around of read-index
  for (size_t initialRdIdx = 0U; initialRdIdx <= capacity; ++initialRdIdx)
  {
    // 2nd varibale of test: Initial write-index
    // Note: "<=" results in intentional wrap around of write-index
    for (size_t initialWrIdx = 0U; initialWrIdx <= capacity; ++initialWrIdx)
    {
      // calculate the initial number of elements in the UUT after the precondition has been setup
      size_t const initialNbOfElements =   (initialRdIdx <= initialWrIdx) ?
                                           (initialWrIdx - initialRdIdx)
                                         : ((capacity + initialWrIdx) - initialRdIdx);

      // calculate the number of free slots in the UUT after the precondition has been setup
      size_t const freeSlots = capacity - initialNbOfElements;

      if (popValue != pushValue)
        throw std::logic_error("Testcase internal error");

      FixCapFIFO<uint32_t, uint8_t> uut(capacity);

      // Setup test precondition:
      // Move read- and write-index into position using whitebox knowledge
      if (initialRdIdx <= initialWrIdx)
      {
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
      }
      else
      {
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, capacity, pushValue));
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
        ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
      }

      ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
      ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
      ASSERT_EQ(uut.Size(), initialNbOfElements);

      // Preconditon established:
      // - read index is (initialRdIdx % capacity) now
      // - write index is (initialWrIdx % capacity) now

      // 3rd variable of test: Copy-assign content of uut to uut2 and push/pop 0..freeSlots elements to/from uut2
      for (size_t n = 0U; n <= freeSlots; ++n)
      {
        // copy-assign
        uut2 = uut;

        // the copy uses its own sequence of values
        uint32_t copyOfPopValue = popValue;
        uint32_t copyOfPushValue = pushValue;

        // check status of uut2
        ASSERT_EQ(uut2.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut2.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut2.Size(), initialNbOfElements);
        ASSERT_EQ(uut2.Capacity(), capacity);

        if (n != 0U)
        {
          // push n values, check FIFO state, pop n values, check FIFO state
          ASSERT_NO_FATAL_FAILURE(UsePush(uut2, n, copyOfPushValue));

          size_t const expectedSize = initialNbOfElements + n;
          ASSERT_EQ(uut2.IsEmpty(), (expectedSize == 0U));
          ASSERT_EQ(uut2.IsFull(), (expectedSize == capacity));
          ASSERT_EQ(uut2.Size(), expectedSize);

          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut2, n, copyOfPopValue));

          ASSERT_EQ(uut2.IsEmpty(), (initialNbOfElements == 0U));
          ASSERT_EQ(uut2.IsFull(), (initialNbOfElements == capacity));
          ASSERT_EQ(uut2.Size(), initialNbOfElements);
        }

        // We do not pop 'initialNbOfElements' elements from uut2 by intention. uut2 is therefore left in non-empty
        // state in some constellations of initialRdIdx and initialWrIdx.
      } // for n...

      // Finally check status and content of UUT. It should not have been changed by creating all the copies.
      ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
      ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
      ASSERT_EQ(uut.Size(), initialNbOfElements);

      ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialNbOfElements, popValue));

      ASSERT_TRUE(uut.IsEmpty());
      ASSERT_FALSE(uut.IsFull());
      ASSERT_EQ(uut.Size(), 0U);
    } // for initialWrIdx...
  } // for initialRdIdx...
}

TEST(gpcc_container_FixCapFIFO_Tests, MoveAssignmentSelf)
{
  FixCapFIFO<uint32_t, uint8_t> uut(2U);
  ASSERT_TRUE(uut.Push(1U));
  ASSERT_TRUE(uut.Push(3U));

  GPCC_DISABLE_WARN_SELFMOVE();
  uut = std::move(uut);
  GPCC_RESTORE_WARN_SELFMOVE();

  ASSERT_EQ(uut.Size(), 2U);
  uint32_t v = 0U;
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 1U);
  ASSERT_TRUE(uut.Pop(v));
  EXPECT_EQ(v, 3U);
}

TEST(gpcc_container_FixCapFIFO_Tests, MoveAssignment)
{
  // This test generates all possible states of the UUT using nested loops and tests move-assignment of the UUT's
  // content to a 2nd instance from each state. The UUT's state is comprised of the index of the read-pointer and the
  // write-pointer. The 2nd instance has either the same or twice the capacity of the 1st instance.

  // counter for creating unique values pushed onto the UUT during this test and for checking the popped values
  uint32_t pushValue = 0U;
  uint32_t popValue = 0U;

  size_t const capacity = 8U;

  // 1st variable of test: Initial read-index
  // Note: "<=" results in intentional wrap around of read-index
  for (size_t initialRdIdx = 0U; initialRdIdx <= capacity; ++initialRdIdx)
  {
    // 2nd varibale of test: Initial write-index
    // Note: "<=" results in intentional wrap around of write-index
    for (size_t initialWrIdx = 0U; initialWrIdx <= capacity; ++initialWrIdx)
    {
      // calculate the initial number of elements in the UUT after the precondition has been setup
      size_t const initialNbOfElements =   (initialRdIdx <= initialWrIdx) ?
                                           (initialWrIdx - initialRdIdx)
                                         : ((capacity + initialWrIdx) - initialRdIdx);

      // calculate the number of free slots in the UUT after the precondition has been setup
      size_t const freeSlots = capacity - initialNbOfElements;

      // 3rd variable of test: Number of elements pushed/popped to/from the FIFO after move-assignment
      for (size_t n = 0U; n <= freeSlots; ++n)
      {
        if (popValue != pushValue)
          throw std::logic_error("Testcase internal error");

        FixCapFIFO<uint32_t, uint8_t> uut(capacity);

        // Setup test precondition:
        // Move read- and write-index into position using whitebox knowledge
        if (initialRdIdx <= initialWrIdx)
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
        }
        else
        {
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, capacity, pushValue));
          ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut, initialRdIdx, popValue));
          ASSERT_NO_FATAL_FAILURE(UsePush(uut, initialWrIdx, pushValue));
        }

        ASSERT_EQ(uut.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut.Size(), initialNbOfElements);

        // Preconditon established:
        // - read index is (initialRdIdx % capacity) now
        // - write index is (initialWrIdx % capacity) now

        // Setup 2nd uut: Capacity same or twice of 1st uut (depends on n), and push some content.
        FixCapFIFO<uint32_t, uint8_t> uut2(capacity * ((n % 2U) + 1U));
        ASSERT_TRUE(uut2.Push(0U));
        ASSERT_TRUE(uut2.Push(8U));
        ASSERT_TRUE(uut2.Push(15U));

        // Test: Move-assign content of uut to uut2
        uut2 = std::move(uut);

        // check state of uut after move-assignment
        ASSERT_TRUE(uut.IsEmpty());
        ASSERT_EQ(uut.Size(), 0U);
        ASSERT_GE(uut.Capacity(), 1U);

        // Test: Use uut2: Push n values, check FIFO state, pop n values, check FIFO state
        ASSERT_NO_FATAL_FAILURE(UsePush(uut2, n, pushValue));

        size_t const expectedSize = initialNbOfElements + n;
        ASSERT_EQ(uut2.IsEmpty(), (expectedSize == 0U));
        ASSERT_EQ(uut2.IsFull(), (expectedSize == capacity));
        ASSERT_EQ(uut2.Size(), expectedSize);

        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut2, n, popValue));

        ASSERT_EQ(uut2.IsEmpty(), (initialNbOfElements == 0U));
        ASSERT_EQ(uut2.IsFull(), (initialNbOfElements == capacity));
        ASSERT_EQ(uut2.Size(), initialNbOfElements);

        // Pop and check 'initialNbOfElements' values from uut2 due to the precondition
        ASSERT_NO_FATAL_FAILURE(UsePopAndCheckValues(uut2, initialNbOfElements, popValue));

        ASSERT_TRUE(uut2.IsEmpty());
        ASSERT_FALSE(uut2.IsFull());
        ASSERT_EQ(uut2.Size(), 0U);
      } // for n...
    } // for initialWrIdx...
  } // for initialRdIdx...
}

} // namespace container
} // namespace gpcc_tests
