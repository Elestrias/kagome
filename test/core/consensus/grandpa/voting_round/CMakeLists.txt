#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(voting_round_test
    voting_round_test.cpp
    )

target_link_libraries(voting_round_test
    voting_round
    vote_graph
    vote_tracker
    blob
    clock
    logger_for_tests
    p2p::p2p_basic_scheduler
    )
