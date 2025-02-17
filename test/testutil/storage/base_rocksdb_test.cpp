/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "testutil/storage/base_rocksdb_test.hpp"

namespace test {

  void BaseRocksDB_Test::open() {
    rocksdb::Options options;
    options.create_if_missing = true;

    auto r = RocksDB::create(getPathString(), options);
    if (!r) {
      throw std::invalid_argument(r.error().message());
    }

    db_ = std::move(r.value());
    ASSERT_TRUE(db_) << "BaseRocksDB_Test: db is nullptr";
  }

  BaseRocksDB_Test::BaseRocksDB_Test(fs::path path)
      : BaseFS_Test(std::move(path)) {}

  void BaseRocksDB_Test::SetUp() {
    open();
  }

  void BaseRocksDB_Test::TearDown() {
    clear();
  }
}  // namespace test
