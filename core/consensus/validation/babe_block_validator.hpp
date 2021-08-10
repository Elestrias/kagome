/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef KAGOME_BABE_BLOCK_VALIDATOR_HPP
#define KAGOME_BABE_BLOCK_VALIDATOR_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "blockchain/block_tree.hpp"
#include "consensus/babe/types/babe_block_header.hpp"
#include "consensus/babe/types/seal.hpp"
#include "consensus/validation/block_validator.hpp"
#include "crypto/hasher.hpp"
#include "crypto/vrf_provider.hpp"
#include "log/logger.hpp"
#include "primitives/authority.hpp"
#include "primitives/babe_configuration.hpp"
#include "primitives/transcript.hpp"
#include "runtime/tagged_transaction_queue.hpp"

namespace kagome::crypto {
  class Sr25519Provider;
}

namespace kagome::consensus {
  /**
   * Validation of blocks in BABE system. Based on the algorithm described here:
   * https://research.web3.foundation/en/latest/polkadot/BABE/Babe/#2-normal-phase
   */
  class BabeBlockValidator : public BlockValidator {
   public:
    ~BabeBlockValidator() override = default;

    /**
     * Create an instance of BabeBlockValidator
     * @param block_tree to be used by this instance
     * @param tx_queue to validate the extrinsics
     * @param hasher to take hashes
     * @param vrf_provider for VRF-specific operations
     * @param configuration Babe configuration from genesis
     */
    BabeBlockValidator(
        std::shared_ptr<blockchain::BlockTree> block_tree,
        std::shared_ptr<runtime::TaggedTransactionQueue> tx_queue,
        std::shared_ptr<crypto::Hasher> hasher,
        std::shared_ptr<crypto::VRFProvider> vrf_provider,
        std::shared_ptr<crypto::Sr25519Provider> sr25519_provider,
        std::shared_ptr<primitives::BabeConfiguration> configuration);

    enum class ValidationError {
      NO_AUTHORITIES = 1,
      INVALID_SIGNATURE,
      INVALID_VRF,
      TWO_BLOCKS_IN_SLOT,
      SECONDARY_SLOT_ASSIGNMENTS_DISABLED
    };

    outcome::result<void> validateHeader(
        const primitives::BlockHeader &header,
        const EpochNumber epoch_number,
        const primitives::AuthorityId &authority_id,
        const Threshold &threshold,
        const Randomness &randomness) const override;

   private:
    /**
     * Verify that block is signed by valid signature
     * @param header Header to be checked
     * @param babe_header BabeBlockHeader corresponding to (fetched from) header
     * @param seal Seal corresponding to (fetched from) header
     * @param public_key public key that corresponds to the authority by
     * authority index
     * @return true if signature is valid, false otherwise
     */
    bool verifySignature(const primitives::BlockHeader &header,
                         const BabeBlockHeader &babe_header,
                         const Seal &seal,
                         const primitives::BabeSessionKey &public_key) const;

    /**
     * Verify that vrf value contained in babe_header is less than threshold and
     * was generated by the creator of the block
     * @param babe_header BabeBlockHeader corresponding to (fetched from) header
     * @param public_key Public key of creator of the block
     * @param threshold threshold value for that epoch
     * @param randomness randomness for that epoch
     * @return true if vrf is valid, false otherwise
     */
    bool verifyVRF(const BabeBlockHeader &babe_header,
                   const EpochNumber epoch_number,
                   const primitives::BabeSessionKey &public_key,
                   const Threshold &threshold,
                   const Randomness &randomness,
                   const bool checkThreshold) const;

    std::shared_ptr<blockchain::BlockTree> block_tree_;
    mutable std::unordered_map<BabeSlotNumber,
                               std::unordered_set<primitives::AuthorityIndex>>
        blocks_producers_;

    std::shared_ptr<runtime::TaggedTransactionQueue> tx_queue_;

    std::shared_ptr<crypto::Hasher> hasher_;

    std::shared_ptr<crypto::VRFProvider> vrf_provider_;
    std::shared_ptr<crypto::Sr25519Provider> sr25519_provider_;

    std::shared_ptr<primitives::BabeConfiguration> configuration_;
    log::Logger log_;
  };
}  // namespace kagome::consensus

OUTCOME_HPP_DECLARE_ERROR(kagome::consensus,
                          BabeBlockValidator::ValidationError)

#endif  // KAGOME_BABE_BLOCK_VALIDATOR_HPP
