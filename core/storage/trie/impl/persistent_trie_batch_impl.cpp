/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/trie/impl/persistent_trie_batch_impl.hpp"

#include <memory>

#include "storage/trie/impl/topper_trie_batch_impl.hpp"
#include "storage/trie/polkadot_trie/polkadot_trie_cursor_impl.hpp"
#include "storage/trie/polkadot_trie/trie_error.hpp"

OUTCOME_CPP_DEFINE_CATEGORY(kagome::storage::trie,
                            PersistentTrieBatchImpl::Error,
                            e) {
  using E = kagome::storage::trie::PersistentTrieBatchImpl::Error;
  switch (e) {
    case E::NO_TRIE:
      return "Trie was not created or already was destructed.";
  }
  return "Unknown error";
}

namespace kagome::storage::trie {
  std::unique_ptr<PersistentTrieBatchImpl> PersistentTrieBatchImpl::create(
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      std::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes,
      std::shared_ptr<PolkadotTrie> trie) {
    std::unique_ptr<PersistentTrieBatchImpl> ptr(
        new PersistentTrieBatchImpl(std::move(codec),
                                    std::move(serializer),
                                    std::move(changes),
                                    std::move(trie)));
    return ptr;
  }

  PersistentTrieBatchImpl::PersistentTrieBatchImpl(
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      std::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes,
      std::shared_ptr<PolkadotTrie> trie)
      : codec_{std::move(codec)},
        serializer_{std::move(serializer)},
        changes_{std::move(changes)},
        trie_{std::move(trie)} {
    BOOST_ASSERT(codec_ != nullptr);
    BOOST_ASSERT(serializer_ != nullptr);
    BOOST_ASSERT((changes_.has_value() && changes_.value() != nullptr)
                 or not changes_.has_value());
    BOOST_ASSERT(trie_ != nullptr);
  }

  outcome::result<RootHash> PersistentTrieBatchImpl::commit() {
    OUTCOME_TRY(root, serializer_->storeTrie(*trie_));
    SL_TRACE_FUNC_CALL(logger_, root);
    return std::move(root);
  }

  std::unique_ptr<TopperTrieBatch> PersistentTrieBatchImpl::batchOnTop() {
    return std::make_unique<TopperTrieBatchImpl>(shared_from_this());
  }

  outcome::result<BufferConstRef> PersistentTrieBatchImpl::get(
      const BufferView &key) const {
    return trie_->get(key);
  }

  outcome::result<std::optional<BufferConstRef>>
  PersistentTrieBatchImpl::tryGet(const BufferView &key) const {
    return trie_->tryGet(key);
  }

  std::unique_ptr<PolkadotTrieCursor> PersistentTrieBatchImpl::trieCursor() {
    return std::make_unique<PolkadotTrieCursorImpl>(trie_);
  }

  outcome::result<bool> PersistentTrieBatchImpl::contains(
      const BufferView &key) const {
    return trie_->contains(key);
  }

  bool PersistentTrieBatchImpl::empty() const {
    return trie_->empty();
  }

  outcome::result<std::tuple<bool, uint32_t>>
  PersistentTrieBatchImpl::clearPrefix(const BufferView &prefix,
                                       std::optional<uint64_t> limit) {
    SL_TRACE_VOID_FUNC_CALL(logger_, prefix);
    return trie_->clearPrefix(
        prefix, limit, [&](const auto &key, auto &&) -> outcome::result<void> {
          if (changes_.has_value()) {
            changes_.value()->onRemove(key);
          }
          return outcome::success();
        });
  }

  outcome::result<void> PersistentTrieBatchImpl::put(const BufferView &key,
                                                     const Buffer &value) {
    OUTCOME_TRY(contains, trie_->contains(key));
    bool is_new_entry = not contains;
    auto res = trie_->put(key, value);
    if (res and changes_.has_value()) {
      SL_TRACE_VOID_FUNC_CALL(logger_, key, value);

      changes_.value()->onPut(key, value, is_new_entry);
    }
    return res;
  }

  outcome::result<void> PersistentTrieBatchImpl::put(const BufferView &key,
                                                     Buffer &&value) {
    return put(key, value);  // cannot take possession of value, check the
                             // const-ref version definition
  }

  outcome::result<void> PersistentTrieBatchImpl::remove(const BufferView &key) {
    OUTCOME_TRY(trie_->remove(key));
    if (changes_.has_value()) {
      SL_TRACE_VOID_FUNC_CALL(logger_, key);
      changes_.value()->onRemove(key);
    }
    return outcome::success();
  }

}  // namespace kagome::storage::trie
