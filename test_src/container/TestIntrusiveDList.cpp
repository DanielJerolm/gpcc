/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include <gpcc/container/IntrusiveDList.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "gtest/gtest.h"
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace container  {

using namespace gpcc::container;
using namespace testing;

// Items that can be added to the UUT (IntrusiveDList<Item>).
class Item
{
  friend class IntrusiveDList<Item>;
  friend class gpcc_container_IntrusiveDList_TestsF;

  public:
    uint32_t value;

    inline Item(void) : value(0U), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
    inline explicit Item(uint32_t const _value) : value(_value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
    inline Item(Item const & other) : value(other.value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
    inline Item(Item const && other) noexcept : value(other.value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};

    inline ~Item(void)
    {
      if ((pPrevInIntrusiveDList != nullptr) || (pNextInIntrusiveDList != nullptr))
        gpcc::osal::Panic("Item::~Item: Object still referenced by IntrusiveDList<Item> instance!");
    }

    inline Item& operator=(Item const & rhv) noexcept
    {
      value = rhv.value;
      return *this;
    }

    inline Item& operator=(Item && rhv) noexcept
    {
      value = rhv.value;
      return *this;
    }

    inline bool AreIntrusiveDListPointersNull(void)
    {
      return (pPrevInIntrusiveDList == nullptr) && (pNextInIntrusiveDList == nullptr);
    }

  private:
    // Pointers used to enqueue instances of this class in IntrusiveDList<Item>
    Item* pPrevInIntrusiveDList;
    Item* pNextInIntrusiveDList;
};

// Test fixture.
// - Provides UUT
// - Finally removes any items from UUT and releases them
// - Provides some useful functions
class gpcc_container_IntrusiveDList_TestsF: public Test
{
  public:
    gpcc_container_IntrusiveDList_TestsF(void);
    virtual ~gpcc_container_IntrusiveDList_TestsF(void);

    protected:
      std::unique_ptr<IntrusiveDList<Item>> spUUT;

      void AddThreeItems(void);
      bool VerifyContent(std::vector<uint32_t> const & expectedValues);
      std::vector<Item*> CopyUUTas_std_vec(void);

      void SetUp(void) override;
      void TearDown(void) override;
};

gpcc_container_IntrusiveDList_TestsF::gpcc_container_IntrusiveDList_TestsF(void)
: Test()
, spUUT()
{
  spUUT = std::make_unique<IntrusiveDList<Item>>();
}

gpcc_container_IntrusiveDList_TestsF::~gpcc_container_IntrusiveDList_TestsF(void)
{
  if (spUUT != nullptr)
    spUUT->ClearAndDestroyItems();
}

void gpcc_container_IntrusiveDList_TestsF::AddThreeItems(void)
{
  for (uint_fast8_t i = 0U; i < 3U; ++i)
  {
    auto p = std::make_unique<Item>(i);
    spUUT->push_back(p.get());
    p.release();
  }
}

bool gpcc_container_IntrusiveDList_TestsF::VerifyContent(std::vector<uint32_t> const & expectedValues)
{
  if (expectedValues.size() != spUUT->size())
  {
    ADD_FAILURE() << "Size mismatch! Expected: " << expectedValues.size() << ", actual: " << spUUT->size();
    return false;
  }

  auto       & uut  = *spUUT;
  auto const & cuut = *spUUT;

  // check by forward iteration (normal iterator, non-const uut)
  {
    auto it1 = uut.begin();
    auto it2 = expectedValues.cbegin();
    bool noFurtherItemExpected = false;
    for (; it1 != uut.end(); ++it1)
    {
      if (noFurtherItemExpected)
      {
        ADD_FAILURE() << "There is a element, but pointers between elements suggested that there should be none";
        return false;
      }

      if ((*it1)->value != *it2)
      {
        ADD_FAILURE() << "Content mismatch, detected during forward iteration (normal iterator, non-const uut)";
        return false;
      }

      // check proper linkage
      if (it1 == uut.begin())
      {
        if ((*it1)->pPrevInIntrusiveDList != nullptr)
        {
          ADD_FAILURE() << "Item is linked with a previous item, but it is the first in the list";
          return false;
        }
      }
      else
      {
        if ((*it1)->pPrevInIntrusiveDList == nullptr)
        {
          ADD_FAILURE() << "Invalid linkage between items";
          return false;
        }

        if ((*it1)->pPrevInIntrusiveDList->pNextInIntrusiveDList != *it1)
        {
          ADD_FAILURE() << "Invalid linkage between items";
          return false;
        }
      }

      if ((*it1)->pNextInIntrusiveDList == nullptr)
      {
        noFurtherItemExpected = true;
      }
      else
      {
        if ((*it1)->pNextInIntrusiveDList->pPrevInIntrusiveDList != *it1)
        {
          ADD_FAILURE() << "Invalid linkage between items";
          return false;
        }
      }

      ++it2;
    }
  }

  // check by forward iteration (implicit const iterator, const uut)
  {
    auto it1 = cuut.begin();
    auto it2 = expectedValues.cbegin();
    for (; it1 != cuut.cend(); ++it1)
    {
      if ((*it1)->value != *it2)
      {
        ADD_FAILURE() << "Content mismatch, detected during forward iteration (implicit const iterator, const uut)";
        return false;
      }
      ++it2;
    }
  }

  // check by forward iteration (const iterator, non-const uut)
  {
    auto it1 = uut.cbegin();
    auto it2 = expectedValues.cbegin();
    for (; it1 != uut.cend(); ++it1)
    {
      if ((*it1)->value != *it2)
      {
        ADD_FAILURE() << "Content mismatch, detected during forward iteration (const iterator, non-const uut)";
        return false;
      }
      ++it2;
    }
  }

  // check by forward iteration (const iterator, const uut)
  {
    auto it1 = cuut.cbegin();
    auto it2 = expectedValues.cbegin();
    for (; it1 != cuut.cend(); ++it1)
    {
      if ((*it1)->value != *it2)
      {
        ADD_FAILURE() << "Content mismatch, detected during forward iteration (const iterator, const uut)";
        return false;
      }
      ++it2;
    }
  }

  return true;
}

std::vector<Item*> gpcc_container_IntrusiveDList_TestsF::CopyUUTas_std_vec(void)
{
  std::vector<Item*> copy;
  for (auto e: *spUUT)
    copy.push_back(e);

  return copy;
}

void gpcc_container_IntrusiveDList_TestsF::SetUp(void)
{
}

void gpcc_container_IntrusiveDList_TestsF::TearDown(void)
{
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, CreateDestroy)
{
  EXPECT_EQ(spUUT->size(), 0U);
  EXPECT_TRUE(spUUT->empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveCTOR1)
{
  // variant 1: move construct non-empty list
  AddThreeItems();

  IntrusiveDList<Item> newList(std::move(*spUUT));

  EXPECT_EQ(spUUT->size(), 0U);
  EXPECT_TRUE(spUUT->empty());

  EXPECT_EQ(newList.size(), 3U);
  EXPECT_FALSE(newList.empty());

  uint_fast8_t i = 0;
  for (auto e: newList)
  {
    EXPECT_EQ(e->value, i);
    ++i;
  }

  newList.ClearAndDestroyItems();
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveCTOR2)
{
  // variant 2: move construct empty list

  IntrusiveDList<Item> newList(std::move(*spUUT));

  EXPECT_EQ(spUUT->size(), 0U);
  EXPECT_TRUE(spUUT->empty());

  EXPECT_EQ(newList.size(), 0U);
  EXPECT_TRUE(newList.empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, DestructorReleasesListElements)
{
  AddThreeItems();
  auto prevContent = CopyUUTas_std_vec();

  spUUT.reset();

  for (auto e: prevContent)
  {
    ASSERT_TRUE(e->AreIntrusiveDListPointersNull());
    delete e;
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveAssignment1)
{
  // variant 1: move assign not-empty list to not-empty list

  IntrusiveDList<Item> list;
  ON_SCOPE_EXIT(releaseListContent) { list.ClearAndDestroyItems(); };

  for (uint_fast8_t i = 0U; i < 4U; ++i)
  {
    auto spItem = std::make_unique<Item>(i * 10U);
    list.push_back(spItem.get());
    spItem.release();
  }

  AddThreeItems();
  auto prevContent = CopyUUTas_std_vec();

  *spUUT = std::move(list);

  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0U);

  EXPECT_FALSE(spUUT->empty());
  EXPECT_EQ(spUUT->size(), 4U);

  ASSERT_TRUE(VerifyContent({0U, 10U, 20U, 30U}));

  for (auto e: prevContent)
    delete e;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveAssignment2)
{
  // variant 2: move assign not-empty list to empty list

  IntrusiveDList<Item> list;
  ON_SCOPE_EXIT(releaseListContent) { list.ClearAndDestroyItems(); };

  for (uint_fast8_t i = 0U; i < 4U; ++i)
  {
    auto spItem = std::make_unique<Item>(i * 10U);
    list.push_back(spItem.get());
    spItem.release();
  }

  *spUUT = std::move(list);

  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0U);

  EXPECT_FALSE(spUUT->empty());
  EXPECT_EQ(spUUT->size(), 4U);

  ASSERT_TRUE(VerifyContent({0U, 10U, 20U, 30U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveAssignment3)
{
  // variant 3: move assign empty list to not-empty list

  IntrusiveDList<Item> list;

  AddThreeItems();
  auto prevContent = CopyUUTas_std_vec();

  *spUUT = std::move(list);

  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0U);

  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);

  for (auto e: prevContent)
    delete e;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, MoveAssignment4)
{
  // variant 3: move assign empty list to empty list

  IntrusiveDList<Item> list;

  *spUUT = std::move(list);

  EXPECT_TRUE(list.empty());
  EXPECT_EQ(list.size(), 0U);

  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration1)
{
  AddThreeItems();
  ASSERT_TRUE(VerifyContent({0U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration2)
{
  AddThreeItems();

  uint_fast8_t i = 0;
  for (auto it = spUUT->begin(); it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }

  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration_emptyContainer)
{
  auto & uut = *spUUT;
  auto const & cuut = *spUUT;

  {
    auto it = uut.begin();
    EXPECT_TRUE(it == uut.end());
  }

  {
    auto it = uut.cbegin();
    EXPECT_TRUE(it == uut.cend());
  }

  {
    auto it = cuut.begin();
    EXPECT_TRUE(it == cuut.end());
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration_NonConstIt_ModifyReferencedObject)
{
  AddThreeItems();

  for (auto it = spUUT->begin(); it != spUUT->end(); ++it)
  {
    (*it)->value++;
  }

  ASSERT_TRUE(VerifyContent({1U, 2U, 3U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration_ConstIt_ModifyReferencedObject)
{
  AddThreeItems();

  for (auto it = spUUT->cbegin(); it != spUUT->cend(); ++it)
  {
    (*it)->value++;
  }

  ASSERT_TRUE(VerifyContent({1U, 2U, 3U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Iteration_MixIteratorAndConstIterator)
{
  AddThreeItems();

  uint_fast8_t i = 0;
  for (auto it = spUUT->begin(); it != spUUT->cend(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (auto it = spUUT->cbegin(); it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, iterator_StdCTOR)
{
  AddThreeItems();

  IntrusiveDList<Item>::iterator it;
  it = spUUT->begin();

  uint_fast8_t i = 0;
  for (; it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, const_iterator_StdCTOR)
{
  AddThreeItems();

  IntrusiveDList<Item>::const_iterator it;
  it = spUUT->cbegin();

  uint_fast8_t i = 0;
  for (; it != spUUT->cend(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, const_iterator_copyConstructFromIterator)
{
  AddThreeItems();

  auto it = spUUT->begin();

  IntrusiveDList<Item>::const_iterator cit(it);

  uint_fast8_t i = 0;
  for (; cit != spUUT->cend(); ++cit)
  {
    EXPECT_EQ((*cit)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (; it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, const_iterator_moveConstructFromIterator)
{
  AddThreeItems();

  auto it = spUUT->begin();

  IntrusiveDList<Item>::const_iterator cit(std::move(it));

  uint_fast8_t i = 0;
  for (; cit != spUUT->cend(); ++cit)
  {
    EXPECT_EQ((*cit)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (; it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, const_iterator_copyAssignFromIterator)
{
  AddThreeItems();

  auto it = spUUT->begin();

  auto cit = spUUT->cbegin();
  ++cit;

  ASSERT_EQ((*cit)->value, 1U);

  cit = it;

  uint_fast8_t i = 0;
  for (; cit != spUUT->cend(); ++cit)
  {
    EXPECT_EQ((*cit)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (; it != spUUT->end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, const_iterator_moveAssignFromIterator)
{
  AddThreeItems();

  auto it = spUUT->begin();

  auto cit = spUUT->cbegin();
  ++cit;

  ASSERT_EQ((*cit)->value, 1U);

  cit = std::move(it);

  uint_fast8_t i = 0;
  for (; cit != spUUT->cend(); ++cit)
  {
    EXPECT_EQ((*cit)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, OperationsOnInvalidIterators)
{
  auto it = spUUT->end();

  ASSERT_THROW(++it, std::logic_error);
  ASSERT_THROW(it++, std::logic_error);
  ASSERT_THROW((void)(*it), std::logic_error);

  EXPECT_TRUE(it == spUUT->end());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, OperationsOnInvalidConstIterators)
{
  auto it = spUUT->cend();

  ASSERT_THROW(++it, std::logic_error);
  ASSERT_THROW(it++, std::logic_error);
  ASSERT_THROW((void)(*it), std::logic_error);

  EXPECT_TRUE(it == spUUT->cend());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, ForEach)
{
  AddThreeItems();

  IntrusiveDList<Item>       & uut = *spUUT;
  IntrusiveDList<Item> const & cuut = *spUUT;
  uint_fast8_t i;

  i = 0;
  for (auto e: uut)
  {
    EXPECT_EQ(e->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (auto const e: uut)
  {
    EXPECT_EQ(e->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (auto e: cuut)
  {
    EXPECT_EQ(e->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  i = 0;
  for (auto const e: cuut)
  {
    EXPECT_EQ(e->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, ForEach_Modify)
{
  AddThreeItems();

  IntrusiveDList<Item> & uut = *spUUT;
  uint_fast8_t i = 5U;
  for (auto e: uut)
  {
    e->value = i++;
  }

  ASSERT_TRUE(VerifyContent({5U, 6U, 7U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Clear)
{
  AddThreeItems();

  EXPECT_EQ(spUUT->size(), 3U);
  EXPECT_FALSE(spUUT->empty());

  auto backup = CopyUUTas_std_vec();

  spUUT->clear();

  ASSERT_EQ(spUUT->size(), 0U);
  ASSERT_TRUE(spUUT->empty());

  for (auto e: backup)
  {
    ASSERT_TRUE(e->AreIntrusiveDListPointersNull());
    delete e;
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_EmptyList)
{
  auto p = std::make_unique<Item>(0);

  spUUT->insert(spUUT->end(), p.get());

  ASSERT_FALSE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 1U);

  EXPECT_EQ(spUUT->back()->value, 0U);
  p.release();
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_AtFront)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  spUUT->insert(spUUT->begin(), p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({3U, 0U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_Middle)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  auto it = spUUT->begin();
  ++it;
  spUUT->insert(it, p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({0U, 3U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_Append)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  spUUT->insert(spUUT->end(), p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({0U, 1U, 2U, 3U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_ConstIt_EmptyList)
{
  auto p = std::make_unique<Item>(0);

  spUUT->insert(spUUT->cend(), p.get());

  ASSERT_FALSE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 1U);

  EXPECT_EQ(spUUT->back()->value, 0U);
  p.release();
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_ConstIt_AtFront)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  spUUT->insert(spUUT->cbegin(), p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({3U, 0U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_ConstIt_Middle)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  auto it = spUUT->cbegin();
  ++it;
  spUUT->insert(it, p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({0U, 3U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Insert_ConstIt_Append)
{
  AddThreeItems();

  auto p = std::make_unique<Item>(3);
  spUUT->insert(spUUT->cend(), p.get());

  ASSERT_EQ(spUUT->size(), 4U);
  p.release();

  ASSERT_TRUE(VerifyContent({0U, 1U, 2U, 3U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Front)
{
  AddThreeItems();

  auto it = spUUT->begin();
  Item* const pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_EQ((*new_it)->value, 1U);
  EXPECT_TRUE(VerifyContent({1U, 2U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());

  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Mid)
{
  AddThreeItems();

  auto it = spUUT->begin();
  ++it;
  Item* const  pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_EQ((*new_it)->value, 2U);
  EXPECT_TRUE(VerifyContent({0U, 2U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());

  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Last)
{
  AddThreeItems();

  auto it = spUUT->begin();
  ++it;
  ++it;
  Item* const  pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_TRUE(new_it == spUUT->end());
  EXPECT_TRUE(VerifyContent({0U, 1U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());

  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_AllFrontToBack)
{
  AddThreeItems();

  auto backup = CopyUUTas_std_vec();

  std::vector<uint32_t> expectedContent = {0U, 1U, 2U};
  uint_fast8_t i = 0;
  auto it = spUUT->begin();
  while (it != spUUT->end())
  {
    ASSERT_NE(i, 4U) << "Attempt to erase fourth item, but there are only three";

    it = spUUT->erase(it);
    i++;

    ASSERT_FALSE(expectedContent.empty());
    expectedContent.erase(expectedContent.begin());

    VerifyContent(expectedContent);
  }

  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);

  for (auto e: backup)
  {
    ASSERT_TRUE(e->AreIntrusiveDListPointersNull());
    delete e;
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_AllBackToFront)
{
  AddThreeItems();

  auto backup = CopyUUTas_std_vec();

  auto it1 = spUUT->begin();
  auto it2 = it1;
  ++it2;
  auto it3 = it2;
  ++it3;

  auto it = spUUT->erase(it3);
  ASSERT_TRUE(it == spUUT->end());
  EXPECT_TRUE(VerifyContent({0U, 1U}));

  it = spUUT->erase(it2);
  ASSERT_TRUE(it == spUUT->end());
  EXPECT_TRUE(VerifyContent({0U}));

  it = spUUT->erase(it1);
  ASSERT_TRUE(it == spUUT->end());
  EXPECT_TRUE(VerifyContent({}));

  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);

  for (auto e: backup)
  {
    ASSERT_TRUE(e->AreIntrusiveDListPointersNull());
    delete e;
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Const_Front)
{
  AddThreeItems();

  auto it = spUUT->begin();
  Item* const  pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_EQ((*new_it)->value, 1U);
  EXPECT_TRUE(VerifyContent({1U, 2U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());
  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Const_Mid)
{
  AddThreeItems();

  auto it = spUUT->begin();
  ++it;
  Item* const  pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_EQ((*new_it)->value, 2U);
  EXPECT_TRUE(VerifyContent({0U, 2U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());
  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, Erase_Const_Last)
{
  AddThreeItems();

  auto it = spUUT->begin();
  ++it;
  ++it;
  Item* const  pVictim = *it;
  auto new_it = spUUT->erase(it);

  EXPECT_TRUE(new_it == spUUT->end());
  EXPECT_TRUE(VerifyContent({0U, 1U}));
  ASSERT_TRUE(pVictim->AreIntrusiveDListPointersNull());
  delete pVictim;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, FrontAndBack)
{
  AddThreeItems();

  IntrusiveDList<Item> & uut = *spUUT;
  IntrusiveDList<Item> const & cuut = *spUUT;

  EXPECT_EQ(uut.front()->value, 0U);
  EXPECT_EQ(cuut.front()->value, 0U);
  EXPECT_EQ(uut.back()->value, 2U);
  EXPECT_EQ(cuut.back()->value, 2U);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, pushBack)
{
  for (uint_fast8_t i = 0; i < 3; ++i)
  {
    auto p = std::make_unique<Item>(i);
    spUUT->push_back(p.get());
    p.release();
  }

  EXPECT_TRUE(VerifyContent({0U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, pushFront)
{
  for (uint_fast8_t i = 0; i < 3; ++i)
  {
    auto p = std::make_unique<Item>(i);
    spUUT->push_front(p.get());
    p.release();
  }

  EXPECT_TRUE(VerifyContent({2U, 1U, 0U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, popBack)
{
  AddThreeItems();
  Item* p;

  p = spUUT->back();
  spUUT->pop_back();
  ASSERT_TRUE(VerifyContent({0U, 1U}));
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;

  p = spUUT->back();
  spUUT->pop_back();
  ASSERT_TRUE(VerifyContent({0U}));
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;

  p = spUUT->back();
  spUUT->pop_back();
  ASSERT_TRUE(VerifyContent({}));
  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, popFront)
{
  AddThreeItems();
  Item* p;

  p = spUUT->front();
  spUUT->pop_front();
  ASSERT_TRUE(VerifyContent({1U, 2U}));
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;

  p = spUUT->front();
  spUUT->pop_front();
  ASSERT_TRUE(VerifyContent({2U}));
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;

  p = spUUT->front();
  spUUT->pop_front();
  ASSERT_TRUE(VerifyContent({}));
  ASSERT_TRUE(spUUT->empty());
  ASSERT_EQ(spUUT->size(), 0U);
  ASSERT_TRUE(p->AreIntrusiveDListPointersNull());
  delete p;
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, SizeAndEmpty)
{
  EXPECT_EQ(spUUT->size(), 0U);
  EXPECT_TRUE(spUUT->empty());

  AddThreeItems();

  EXPECT_EQ(spUUT->size(), 3U);
  EXPECT_FALSE(spUUT->empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, ClearAndDestroyItems_0)
{
  spUUT->ClearAndDestroyItems();
  EXPECT_TRUE(spUUT->empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, ClearAndDestroyItems_1)
{
  auto p = std::make_unique<Item>(1);
  spUUT->push_back(p.get());
  p.release();

  spUUT->ClearAndDestroyItems();
  EXPECT_TRUE(spUUT->empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, ClearAndDestroyItems_3)
{
  AddThreeItems();

  spUUT->ClearAndDestroyItems();
  EXPECT_TRUE(spUUT->empty());
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, AttemptToAddItemWhichIsAlreadyInList)
{
  AddThreeItems();

  auto p = spUUT->front();

  ASSERT_THROW(spUUT->push_back(p), std::logic_error);
  ASSERT_THROW(spUUT->push_front(p), std::logic_error);
  ASSERT_THROW(spUUT->insert(spUUT->begin(), p), std::logic_error);
  ASSERT_THROW(spUUT->insert(spUUT->cbegin(), p), std::logic_error);

  ASSERT_TRUE(VerifyContent({0U, 1U, 2U}));
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, std_list)
{
  std::list<Item*> std_list;
  ON_SCOPE_EXIT(release_std_list_content)
  {
    for (auto e: std_list)
      delete e;
  };

  for (uint_fast8_t i = 0U; i < 3U; ++i)
  {
    auto spItem = std::make_unique<Item>(i);
    std_list.push_back(spItem.get());
    spItem.release();
  }

  uint_fast8_t i = 0;
  for (auto it = std_list.begin(); it != std_list.end(); ++it)
  {
    EXPECT_EQ((*it)->value, i);
    ++i;
  }
  EXPECT_EQ(i, 3U);

  EXPECT_EQ(std_list.front()->value, 0U);
  EXPECT_EQ(std_list.back()->value, 2U);
}

template<class T>
void InteroperabilityTest(T & uut)
{
  // This is intented to test if IntrusiveDList<A> is a replacement for std::list<A*>.
  // The tests InteroperabilityTest1 and InteroperabilityTest2 will invoke this with T being a std::list<Item*> and
  // T being a IntrusiveDList<Item>.

  T const & cuut = uut;

  // size/empty (list empty) --------------------------------------------------
  EXPECT_TRUE(uut.empty());
  EXPECT_EQ(uut.size(), 0U);

  // push_back ----------------------------------------------------------------
  for (uint_fast8_t i = 3U; i < 6U; ++i)
  {
    auto spItem = std::make_unique<Item>(i);
    uut.push_back(spItem.get());
    spItem.release();
  }

  // push_front ---------------------------------------------------------------
  for (uint_fast8_t i = 3U; i > 0U; --i)
  {
    auto spItem = std::make_unique<Item>(i-1);
    uut.push_front(spItem.get());
    spItem.release();
  }

  // size/empty (list not empty) ----------------------------------------------
  EXPECT_FALSE(uut.empty());
  EXPECT_EQ(uut.size(), 6U);

  // front/back ---------------------------------------------------------------
  EXPECT_EQ(uut.front()->value, 0U);
  EXPECT_EQ(uut.back()->value, 5U);

  // pop_front / pop_back -----------------------------------------------------
  {
    auto p = uut.front();
    uut.pop_front();
    delete p;

    p = uut.back();
    uut.pop_back();
    delete p;

    EXPECT_EQ(uut.front()->value, 1U);
    EXPECT_EQ(uut.back()->value, 4U);
  }

  // insert (non-const iterator) ----------------------------------------------
  {
    auto spItem = std::make_unique<Item>(10U);
    auto it = uut.begin();
    ++it;

    auto new_it = uut.insert(it, spItem.get());
    spItem.release();

    EXPECT_EQ((*new_it)->value, 10U);
  }

  // insert (const iterator) --------------------------------------------------
  {
    auto spItem = std::make_unique<Item>(11U);
    auto it = uut.cbegin();
    ++it;

    auto new_it = uut.insert(it, spItem.get());
    spItem.release();

    EXPECT_EQ((*new_it)->value, 11U);
  }

  // expected content: 1 11 10 2 3 4

  // modify referenced object (non-const iterator) ----------------------------
  {
    auto it = uut.begin();
    ++it;
    (*it)->value++;
  }

  // modify referenced object (const uut, const iterator) ---------------------
  {
    auto it = cuut.cbegin();
    ++it;
    ++it;
    (*it)->value++;
  }

  // expected content: 1 12 11 2 3 4

  // erase (non-const iterator) -----------------------------------------------
  {
    auto it = uut.begin();
    ++it;
    ++it;
    ++it;
    ++it;

    auto p = *it;
    auto new_it = uut.erase(it);
    delete p;

    EXPECT_EQ((*new_it)->value, 4U);
  }

  // erase (const iterator) ---------------------------------------------------
  {
    auto it = uut.cbegin();
    ++it;
    ++it;
    ++it;
    ++it;

    auto p = *it;
    auto new_it = uut.erase(it);
    delete p;

    EXPECT_TRUE(new_it == uut.cend());
  }

  std::vector<uint32_t> const expectedContent = {1, 12, 11, 2};

  // iteration (for each, uut non-const, e non-const) -------------------------
  {
    auto it = expectedContent.begin();
    for (auto e: uut)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, e->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  // iteration (for each, uut const, e non-const) -----------------------------
  {
    auto it = expectedContent.begin();
    for (auto e: cuut)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, e->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  // iteration (for each, uut non-const, e const) -----------------------------
  {
    auto it = expectedContent.begin();
    for (auto const e: uut)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, e->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  // iteration (for each, uut const, e const) -----------------------------
  {
    auto it = expectedContent.begin();
    for (auto const e: cuut)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, e->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  // iteration (manual, different variants const/non-const) -------------------
  {
    auto it = expectedContent.begin();
    for (auto i = uut.begin(); i != uut.end(); ++i)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, (*i)->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  {
    auto it = expectedContent.begin();
    for (auto i = uut.cbegin(); i != uut.cend(); ++i)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, (*i)->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  {
    auto it = expectedContent.begin();
    for (auto i = uut.begin(); i != uut.cend(); ++i)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, (*i)->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  {
    auto it = expectedContent.begin();
    for (auto i = cuut.begin(); i != cuut.end(); ++i)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, (*i)->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  {
    auto it = expectedContent.begin();
    for (auto i = cuut.cbegin(); i != cuut.cend(); ++i)
    {
      if (it == expectedContent.end())
      {
        ADD_FAILURE();
        break;
      }
      EXPECT_EQ(*it, (*i)->value);
      ++it;
    }
    EXPECT_TRUE(it == expectedContent.end());
  }

  // (content is: 1 12 11 2)

  // iterator copy-CTOR -------------------------------------------------------
  {
    auto it = uut.begin();
    auto it2(it);
    EXPECT_EQ((*it2)->value, 1U);
  }

  // iterator move-CTOR -------------------------------------------------------
  {
    auto it = uut.begin();
    auto it2(std::move(it));
    EXPECT_EQ((*it2)->value, 1U);
  }

    // iterator copy-assignment -----------------------------------------------
  {
    auto it = uut.begin();
    auto it2 = uut.end();
    it2 = it;
    EXPECT_EQ((*it2)->value, 1U);
  }

    // iterator move-assignment -----------------------------------------------
  {
    auto it = uut.begin();
    auto it2 = uut.end();
    it2 = std::move(it);
    EXPECT_EQ((*it2)->value, 1U);
  }
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, InteroperabilityTest1)
{
  std::list<Item*> std_list;
  ON_SCOPE_EXIT()
  {
    for (auto e: std_list)
      delete e;
  };

  InteroperabilityTest(std_list);
}

TEST_F(gpcc_container_IntrusiveDList_TestsF, InteroperabilityTest2)
{
  InteroperabilityTest(*spUUT);
}

} // namespace container
} // namespace gpcc_tests
