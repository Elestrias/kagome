/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "scale/boolean.hpp"
#include "scale/scale_error.hpp"

namespace kagome::scale::boolean {

  void encodeBool(bool value, common::Buffer &out) {
    uint8_t byte = (value ? 0x01 : 0x00);
    out.putUint8(byte);
  }

  outcome::result<bool> decodeBool(common::ByteStream &stream) {
    auto byte = stream.nextByte();
    if (!byte.has_value()) {
      return DecodeError::NOT_ENOUGH_DATA;
    }

    switch (*byte) {
      case 0:
        return outcome::success(false);
      case 1:
        return outcome::success(true);
      default:
        break;
    }

    return DecodeError::UNEXPECTED_VALUE;
  }

  void encodeTribool(tribool value, common::Buffer &out) {
    auto byte = static_cast<uint8_t>(2);

    if (value) {
      byte = static_cast<uint8_t>(1);
    }

    if (!value) {
      byte = static_cast<uint8_t>(0);
    }
    out.putUint8(byte);
  }

  outcome::result<tribool> decodeTribool(common::ByteStream &stream) {
    auto byte = stream.nextByte();
    if (!byte.has_value()) {
      return DecodeError::NOT_ENOUGH_DATA;
    }

    switch (*byte) {
      case 0:
        return false;
      case 1:
        return true;
      case 2:
        return indeterminate;
      default:
        break;
    }

    return DecodeError::UNEXPECTED_VALUE;
  }
}  // namespace kagome::scale::boolean
