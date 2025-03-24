#include <cstdio>
#include <gtest/gtest.h>
#include <fstream>
#include <json/json.h>
#include "storage/index/radix_index.hpp"

class RadixTreeJSONTest : public ::testing::Test
{
protected:
  radix::radix_tree<int> tree;
  Json::Value            op_json;
  Json::Value            data_json;
  Json::Value            expect_json;

  void SetUp() override
  {
    std::ifstream           op_file("/config/workspace/radix_dev/miniob/unittest/observer/radix/test/operation.json");
    Json::CharReaderBuilder reader_builder;
    std::string             errs;
    bool                    success = Json::parseFromStream(reader_builder, op_file, &op_json, &errs);
    if (!success) {
      std::cerr << "Failed to parse operation.json: " << errs << std::endl;
    }
    op_file.close();

    std::ifstream data_file("/config/workspace/radix_dev/miniob/unittest/observer/radix/test/data.json");
    success = Json::parseFromStream(reader_builder, data_file, &data_json, &errs);
    if (!success) {
      std::cerr << "Failed to parse data.json: " << errs << std::endl;
    }
    data_file.close();

    std::ifstream expect_file("/config/workspace/radix_dev/miniob/unittest/observer/radix/test/expect.json");
    success = Json::parseFromStream(reader_builder, expect_file, &expect_json, &errs);
    if (!success) {
      std::cerr << "Failed to parse expect.json: " << errs << std::endl;
    }
    expect_file.close();
  }
};

TEST_F(RadixTreeJSONTest, ExecuteOperationsAndCompareResults)
{
  std::vector<bool> results;
  for (Json::ArrayIndex i = 0; i < op_json.size(); ++i) {
    std::string operation = op_json[i].asString();
    Json::Value data      = data_json[i];

    if (operation == "Trie") {
      results.push_back(false);
    } else if (operation == "insert") {
      std::string key = data[0].asString();
      tree.put(key, 1);
      results.push_back(false);
    } else if (operation == "search") {
      std::string key = data[0].asString();

      auto result = tree.search(key);
      results.push_back(result.has_value());
    } else if (operation == "startsWith") {
      // 这里假设 radix_tree 未实现 startsWith 方法
      GTEST_SKIP() << "startsWith method is not implemented yet.";
    }
  }

  for (Json::ArrayIndex i = 0; i < results.size(); ++i) {
    bool expected = expect_json[i].asString() == "true";
    if (results[i] != expected) {
      std::string operation = op_json[i].asString();
      Json::Value data      = data_json[i];
      std::string dataStr;
      if (data.isArray()) {
        for (Json::ArrayIndex j = 0; j < data.size(); ++j) {
          dataStr += data[j].asString();
          if (j < data.size() - 1) {
            dataStr += ", ";
          }
        }
      }
      GTEST_FAIL() << "Test case " << i << " failed.\n"
                   << "Operation: " << operation << "\n"
                   << "Data: " << dataStr << "\n"
                   << "Expected: " << (expected ? "true" : "false") << "\n"
                   << "Actual: " << (results[i] ? "true" : "false");
    }
  }
}
