/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "storage/trie/impl/persistent_trie_batch_impl.hpp"

#include <memory>

#include "scale/scale.hpp"
#include "storage/predefined_keys.hpp"
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
  // sometimes there is no extrinsic index for a runtime call
  const common::Buffer NO_EXTRINSIC_INDEX_VALUE{
      scale::encode(0xffffffff).value()};

  std::unique_ptr<PersistentTrieBatchImpl> PersistentTrieBatchImpl::create(
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      boost::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes,
      std::shared_ptr<PolkadotTrie> trie,
      RootChangedEventHandler &&handler) {
    std::unique_ptr<PersistentTrieBatchImpl> ptr(
        new PersistentTrieBatchImpl(std::move(codec),
                                    std::move(serializer),
                                    std::move(changes),
                                    std::move(trie),
                                    std::move(handler)));
    return ptr;
  }

  PersistentTrieBatchImpl::PersistentTrieBatchImpl(
      std::shared_ptr<Codec> codec,
      std::shared_ptr<TrieSerializer> serializer,
      boost::optional<std::shared_ptr<changes_trie::ChangesTracker>> changes,
      std::shared_ptr<PolkadotTrie> trie,
      RootChangedEventHandler &&handler)
      : codec_{std::move(codec)},
        serializer_{std::move(serializer)},
        changes_{std::move(changes)},
        trie_{std::move(trie)},
        root_changed_handler_{std::move(handler)} {
    BOOST_ASSERT(codec_ != nullptr);
    BOOST_ASSERT(serializer_ != nullptr);
    BOOST_ASSERT((changes_.has_value() && changes_.value() != nullptr)
                 or not changes_.has_value());
    BOOST_ASSERT(trie_ != nullptr);
  }

  outcome::result<common::Buffer> PersistentTrieBatchImpl::getExtrinsicIndex()
      const {
    auto res = trie_->get(kExtrinsicIndexKey);
    if (res.has_error() and res.error() == TrieError::NO_VALUE) {
      return NO_EXTRINSIC_INDEX_VALUE;
    }
    return res;
  }

  outcome::result<RootHash> PersistentTrieBatchImpl::commit() {
    OUTCOME_TRY(root, serializer_->storeTrie(*trie_));
    root_changed_handler_(root);
    SL_TRACE_FUNC_CALL(logger_, root);
    return std::move(root);
  }

  std::unique_ptr<TopperTrieBatch> PersistentTrieBatchImpl::batchOnTop() {
    return std::make_unique<TopperTrieBatchImpl>(shared_from_this());
  }

  outcome::result<Buffer> PersistentTrieBatchImpl::get(
      const Buffer &key) const {
    return trie_->get(key);
  }

  outcome::result<boost::optional<Buffer>> PersistentTrieBatchImpl::tryGet(
      const Buffer &key) const {
    return trie_->tryGet(key);
  }

  std::unique_ptr<PolkadotTrieCursor> PersistentTrieBatchImpl::trieCursor() {
    return std::make_unique<PolkadotTrieCursorImpl>(*trie_);
  }

  bool PersistentTrieBatchImpl::contains(const Buffer &key) const {
    return trie_->contains(key);
  }

  bool PersistentTrieBatchImpl::empty() const {
    return trie_->empty();
  }

  outcome::result<std::tuple<bool, uint32_t>>
  PersistentTrieBatchImpl::clearPrefix(const Buffer &prefix,
                                       boost::optional<uint64_t> limit) {
    if (changes_.has_value()) changes_.value()->onClearPrefix(prefix);
    SL_TRACE_VOID_FUNC_CALL(logger_, prefix);
    OUTCOME_TRY(ext_idx, getExtrinsicIndex());
    return trie_->clearPrefix(
        prefix, limit, [&](const auto &key, auto &&) -> outcome::result<void> {
          if (changes_.has_value()) {
            OUTCOME_TRY(changes_.value()->onRemove(ext_idx, key));
          }
          return outcome::success();
        });
  }

  outcome::result<void> PersistentTrieBatchImpl::put(const Buffer &key,
                                                     const Buffer &value) {
    bool is_new_entry = not trie_->contains(key);
    auto res = trie_->put(key, value);
    if (res and changes_.has_value()) {
      OUTCOME_TRY(ext_idx, getExtrinsicIndex());
      SL_TRACE_VOID_FUNC_CALL(logger_, key, value);
      OUTCOME_TRY(changes_.value()->onPut(ext_idx, key, value, is_new_entry));
    }
    return res;
  }

  outcome::result<void> PersistentTrieBatchImpl::put(const Buffer &key,
                                                     Buffer &&value) {
    return put(key, value);  // cannot take possession of value, check the
                             // const-ref version definition
  }

  outcome::result<void> PersistentTrieBatchImpl::remove(const Buffer &key) {
    auto res = trie_->remove(key);
    if (res and changes_.has_value()) {
      SL_TRACE_VOID_FUNC_CALL(logger_, key);
      OUTCOME_TRY(ext_idx, getExtrinsicIndex());
      OUTCOME_TRY(changes_.value()->onRemove(ext_idx, key));
    }
    return res;
  }

}  // namespace kagome::storage::trie
