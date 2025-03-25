#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

#include "common/log/log.h"
#include "common/lang/string.h"
#include "common/lang/string_view.h"

namespace radix {

template <typename V>
struct radix_node
{
  string                                      subkey;
  std::shared_ptr<V>                          v;
  std::vector<std::shared_ptr<radix_node<V>>> children;
  size_t                                      children_count = 0;

  inline bool has_child(char ch) const { return children[static_cast<uint8_t>(ch)] != nullptr; }

  inline void add_child(char ch, std::shared_ptr<radix_node<V>> child)
  {
    children[static_cast<uint8_t>(ch)] = child;
    children_count++;
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
  radix_tree() { root_ = std::make_shared<radix_node<V>>(children_count()); }

  static constexpr size_t children_count() { return 256; }

  void               put(string_view key, V val);
  std::shared_ptr<V> remove(string_view key);
  std::optional<V>   search(string_view key);

private:
  void recursive_put(
      std::shared_ptr<radix_node<V>> parent, std::shared_ptr<radix_node<V>> node, string_view key, V val);
  std::pair<std::shared_ptr<V>, bool> recursive_remove(
      std::shared_ptr<radix_node<V>> parent, std::shared_ptr<radix_node<V>> node, string_view key);

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
void radix_tree<V>::put(string_view key, V val)
{
  if (key.empty()) {
    return;
  }

  return recursive_put(root_, root_->children[key[0]], key, val);
}

template <typename V>
std::shared_ptr<V> radix_tree<V>::remove(string_view key)
{
  if (key.empty()) {
    return nullptr;
  }
  return recursive_remove(root_, root_->children[key[0]], key).first;
}

template <typename V>
std::pair<std::shared_ptr<V>, bool> radix_tree<V>::recursive_remove(
    std::shared_ptr<radix_node<V>> parent, std::shared_ptr<radix_node<V>> node, string_view key)
{
  if (node == nullptr || key.empty()) {
    return {nullptr , false};
  }

  auto [matched_prefix_length, latest_matched_ch] = prefix_length(node->subkey, key);
  if (matched_prefix_length == key.size()) {
    if (node->v == nullptr) {
      return {nullptr, false};
    }
    auto res   = node->v;
    bool clean = false;
    node->v    = nullptr;
    if (node->children_count == 0) {
      clean = true;
    }
    return {res, clean};
  } else {
    key                     = key.substr(matched_prefix_length);
    auto [res, child_clean] = recursive_remove(node, node->children[key[0]], key);
    if (child_clean) {
      node->children_count -= 1;
      child_clean = node->children_count == 0;
      node->children[key[0]] = nullptr;
    }
    return {res, child_clean};
  }
}

template <typename V>
std::optional<V> radix_tree<V>::search(string_view key)
{
  if (key.empty()) {
    return std::nullopt;
  }
  auto node = root_->children[key[0]];
  while (key.size() && node) {
    string_view prefix              = node->subkey;
    auto [matched_prefix_length, _] = prefix_length(key, prefix);
    if (matched_prefix_length == key.size()) {
      if (matched_prefix_length == node->subkey.size() && node->v != nullptr) {
        return std::optional<V>{*node->v};
      }
      return std::nullopt;
    } else if (matched_prefix_length == node->subkey.size()) {
      char ch = key.at(matched_prefix_length);
      key     = key.substr(matched_prefix_length);
      node    = node->children[static_cast<uint8_t>(ch)];
    } else {
      return std::nullopt;
    }
  }

  if (node) {
    return node->v == nullptr ? std::nullopt : std::optional<V>{*node->v};
  }
  return std::nullopt;
}

template <typename V>
void radix_tree<V>::recursive_put(
    std::shared_ptr<radix_node<V>> parent, std::shared_ptr<radix_node<V>> node, string_view key, V val)
{
  if (node == nullptr) {
    node = std::make_shared<radix_node<V>>(children_count(), string{key}, std::move(val));
    parent->add_child(key[0], node);
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
      parent->add_child(new_parent->subkey[0], new_parent);

      // printf("totally matched case 2: new_parent{ %s },  node{ %s }\n", new_parent->subkey.c_str(),
      // node->subkey.c_str());
    }
    return;
  } else {
    if (node->subkey.size() == matched_prefix_length) {
      // node: bc, insert bcde
      key     = key.substr(matched_prefix_length);
      char ch = key[0];
      recursive_put(node, node->children[static_cast<uint8_t>(ch)], key, std::move(val));
    } else {
      // split
      // node: bcef, insert bcde
      // node -> ef, new node -> bc, new node -> de
      string node_key       = node->subkey.substr(matched_prefix_length);
      string new_parent_key = node->subkey.substr(0, matched_prefix_length);
      string new_child_key  = string{key.substr(matched_prefix_length)};
      // printf("%s, %s, %s\n", node_key.c_str(), new_parent_key.c_str(), new_child_key.c_str());
      node->subkey    = node_key;
      auto new_parent = std::make_shared<radix_node<V>>(children_count(), std::move(new_parent_key));
      auto new_child  = std::make_shared<radix_node<V>>(children_count(), std::move(new_child_key), std::move(val));
      new_parent->add_child(node->subkey[0], node);
      new_parent->add_child(new_child->subkey[0], new_child);
      // replace
      parent->add_child(new_parent->subkey[0], new_parent);
    }
  }
}

}  // namespace radix