#include "gtest/gtest.h"
#define private public
#include "storage/index/radix_index.hpp"

using namespace radix;

TEST(radix_test, totally_matched_test) {
    radix_tree<int> tree;
    tree.put("bcde", 1);
    tree.put("bc", 2);

    EXPECT_EQ(*tree.root_->v, 2);
    EXPECT_EQ(*tree.root_->children[static_cast<uint8_t>('d')]->v, 1);

    EXPECT_EQ(tree.search("bcde").value(), 1);
    EXPECT_EQ(tree.search("bc").value(), 2);
    EXPECT_EQ(tree.search("b").has_value(), false);
}

int main(int argc, char** argv) {
// 分析gtest程序的命令行参数
  testing::InitGoogleTest(&argc, argv);

  // 调用RUN_ALL_TESTS()运行所有测试用例
  // main函数返回RUN_ALL_TESTS()的运行结果
  return RUN_ALL_TESTS();
}