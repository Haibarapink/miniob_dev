#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <optional>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "common/lang/string_view.h"

namespace radix {

template <typename V>
struct radix_node
{
  bool                                        leaf = false;
  string                                      subkey;
  std::shared_ptr<V>                          v;
  std::vector<std::shared_ptr<radix_node<V>>> children;

  inline bool has_child(char ch) const { return children[static_cast<uint8_t>(ch)] != nullptr; }

  inline void add_child(char ch, std::shared_ptr<radix_node<V>> child)
  {
    ASSERT(!has_child(ch), "");
    children[static_cast<uint8_t>(ch)] = child;
  }

  inline std::shared_ptr<radix_node<V>> &at(char ch) { return children[static_cast<uint8_t>(ch)]; }

  radix_node() = default;

  radix_node(string key, V val)
  {
    subkey = std::move(key);
    v      = std::make_shared<V>(std::move(val));
  }

  radix_node(size_t children_count) { children.resize(children_count); }

  radix_node(size_t children_count, string key)
  {
    children.resize(children_count);
    subkey = std::move(key);
  }

  radix_node(size_t children_count, string key, V val)
  {
    children.resize(children_count);
    subkey = std::move(key);
    v      = std::make_shared<V>(std::move(val));
  }
};

template <typename V>
class radix_tree
{
public:
  radix_tree() { root_ = nullptr; }

  static constexpr size_t children_count() { return 256; }

  void put(const string &key, V val);

  std::optional<V> search(string_view key);

private:
  string_view consume(size_t count, string_view key) { return key.substr(count); }

  void recursive_put(std::shared_ptr<radix_node<V>> &node, string_view key, V val);

private:
  /**
   * @brief return the prefix length of str1 and str2
   */
  std::pair<size_t, char> prefix_length(string_view str1, string_view str2)
  {
    size_t p   = 0;
    char   ch  = 0;
    size_t len = std::min(str1.size(), str2.size());
    for (; p < len && str1.at(p) == str2.at(p); ++p) {
      ch = str1.at(p);
    }
    return {p, ch};
  }

  size_t                         size_;
  std::shared_ptr<radix_node<V>> root_;
};

template <typename V>
void radix_tree<V>::put(const string &key, V val)
{
  if (key.empty()) {
    return;
  }

  return recursive_put(root_, key, val);
}

template <typename V>
std::optional<V> radix_tree<V>::search(string_view key)
{
  auto node = root_;
  while (key.size() && node) {
    string_view prefix              = node->subkey;
    auto [matched_prefix_length, _] = prefix_length(key, prefix);
    if (matched_prefix_length == key.size()) {
      if (matched_prefix_length == node->subkey.size() && node->v != nullptr) {
        return std::optional<V>{*node->v};
      }
      return std::nullopt;
    } else {
      char ch = key.at(matched_prefix_length);
      key     = key.substr(matched_prefix_length);
      node    = node->children[static_cast<uint8_t>(ch)];
    }
  }
  if (node) {
    return node->v == nullptr ? std::nullopt : std::optional<V>{*node->v};
  }
  return std::nullopt;
}

template <typename V>
void radix_tree<V>::recursive_put(std::shared_ptr<radix_node<V>> &node, string_view key, V val)
{
  if (node == nullptr) {
    node = std::make_shared<radix_node<V>>(children_count(), string{key}, std::move(val));
    return;
  }

  if (key.empty()) {
    node->v = std::make_shared<V>(std::move(val));
    return;
  }

  auto [matched_prefix_length, latest_matched_ch] = prefix_length(node->subkey, key);
  if (key.size() == matched_prefix_length) {
    if (matched_prefix_length == node->subkey.size()) {
      // totally matched
      node->v = std::make_shared<V>(std::move(val));
    } else {
      // node: bcde , insert bc
      // old node: de,
      string node_prefix       = node->subkey.substr(matched_prefix_length);
      string new_parent_prefix = node->subkey.substr(0, matched_prefix_length);
      node->subkey             = node_prefix;
      auto new_parent = std::make_shared<radix_node<V>>(children_count(), std::move(new_parent_prefix), std::move(val));
      new_parent->add_child(node->subkey[0], node);
      // replace
      node = new_parent;
    }
    return;
  } else {
  }
}

}  // namespace radix