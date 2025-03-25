#include "gtest/gtest.h"
#include <string>
#define private public
#include "storage/index/radix_index.hpp"

using namespace radix;


TEST(RadixTreeTest, InsertCase0)
{
  radix_tree<int> tree;
  tree.put("e", 1);
  tree.put("bc", 2);

  EXPECT_EQ(tree.search("e").value(), 1);
  EXPECT_EQ(tree.search("bc").value(), 2);
  EXPECT_EQ(tree.search("b").has_value(), false);
}

TEST(RadixTreeTest, InsertCase1)
{
  radix_tree<int> tree;
  tree.put("bcde", 1);
  tree.put("bc", 2);

  EXPECT_EQ(tree.search("bcde").value(), 1);
  EXPECT_EQ(tree.search("bc").value(), 2);
  EXPECT_EQ(tree.search("b").has_value(), false);
}

TEST(RadixTreeTest, InsertCase2)
{
  radix_tree<int> tree;
  tree.put("bc", 1);
  tree.put("bcde", 2);

  EXPECT_EQ(tree.search("bcde").value(), 2);
  EXPECT_EQ(tree.search("bc").value(), 1);
  EXPECT_EQ(tree.search("b").has_value(), false);
}

TEST(RadixTreeTest, InsertCase3)
{
  radix_tree<int> tree;
  tree.put("bcef", 1);
  tree.put("bcde", 2);

  EXPECT_EQ(tree.search("bcef").value(), 1);
  EXPECT_EQ(tree.search("bcde").value(), 2);
  EXPECT_EQ(tree.search("b").has_value(), false);
}

TEST(RadixTreeTest, InsertCase4)
{
  radix_tree<int> tree;
  tree.put("bcef", 1);
  tree.put("bcef", 2);

  EXPECT_EQ(tree.search("bcef").value(), 2);
  EXPECT_EQ(tree.search("b").has_value(), false);
}

TEST(RadixTreeTest, InsertAndSearch)
{
  radix_tree<int> tree;

  tree.put("apple", 1);
  tree.put("banana", 2);
  tree.put("cherry", 3);

  auto result1 = tree.search("apple");
  EXPECT_TRUE(result1.has_value());
  EXPECT_EQ(result1.value(), 1);

  auto result2 = tree.search("banana");
  EXPECT_TRUE(result2.has_value());
  EXPECT_EQ(result2.value(), 2);

  auto result3 = tree.search("cherry");
  EXPECT_TRUE(result3.has_value());
  EXPECT_EQ(result3.value(), 3);

  auto result4 = tree.search("date");
  EXPECT_FALSE(result4.has_value());
}

TEST(RadixTreeTest, InsertCase5)
{
  radix_tree<int> tree;
  for (auto i = 0; i < 10000; ++i) {
    string k = "key" + std::to_string(i);
    tree.put(k, i);
  }

  for (auto i = 0; i < 10000; ++i) {
    string k   = "key" + std::to_string(i);
    auto   res = tree.search(k);
    EXPECT_EQ(res.has_value(), true);
    EXPECT_EQ(res.value(), i);
  }
}

TEST(RadixTreeTest, InsertSameKey)
{
  radix_tree<int> tree;

  tree.put("apple", 1);
  tree.put("apple", 2);

  auto result = tree.search("apple");
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result.value(), 2);
}

TEST(RadixTreeTest, InsertEmptyKey)
{
  radix_tree<int> tree;
  tree.put("", 1);
  auto result = tree.search("");
  EXPECT_FALSE(result.has_value());
}
// 测试 search 方法的边界情况
TEST(RadixTreeTest, SearchBoundaryCase) {
    radix_tree<int> tree;
    tree.put("cherry", 1);
    auto result = tree.search("cherr");
    EXPECT_FALSE(result.has_value());
}

TEST(RadixTreeTest, RemoveCase)
{
  radix_tree<int> tree;
  for (auto i = 0; i < 10000; ++i) {
    string k = "key" + std::to_string(i);
    tree.put(k, i);
  }

  auto v=  *tree.remove("key1234");
  EXPECT_EQ(v, 1234);
  EXPECT_FALSE(tree.search("key1234").has_value());

  
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}