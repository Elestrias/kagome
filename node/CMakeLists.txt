#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_executable(kagome
    main.cpp
    )
target_link_libraries(kagome
    Backward::Backward
    Boost::boost
    Boost::program_options
    kagome_application
    app_config
    fd_limit
    p2p::p2p_identify
    p2p::p2p_ping
    )
kagome_install(kagome)
