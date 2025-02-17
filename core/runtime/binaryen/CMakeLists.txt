#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(binaryen_wasm_memory
    memory_impl.cpp
    )
target_link_libraries(binaryen_wasm_memory
    buffer
    logger
    binaryen::binaryen
    memory_allocator
    )
kagome_install(binaryen_wasm_memory)

add_library(binaryen_wasm_memory_factory binaryen_memory_factory.cpp)
target_link_libraries(binaryen_wasm_memory_factory
    binaryen_wasm_memory
    )
kagome_install(binaryen_wasm_memory_factory)

add_library(binaryen_runtime_external_interface
    runtime_external_interface.hpp
    runtime_external_interface.cpp
    )
target_link_libraries(binaryen_runtime_external_interface
    binaryen::binaryen
    binaryen_wasm_memory
    logger
    )
kagome_install(binaryen_runtime_external_interface)

add_library(binaryen_memory_provider binaryen_memory_provider.cpp)
target_link_libraries(binaryen_memory_provider
    binaryen_wasm_memory_factory
    binaryen_runtime_external_interface
    )
kagome_install(binaryen_memory_provider)

add_library(binaryen_wasm_module
    module/module_impl.cpp
    module/module_instance_impl.cpp
    )
target_link_libraries(binaryen_wasm_module
    binaryen::binaryen
    logger
    binaryen_runtime_external_interface
    trie_storage_provider
    mp_utils
    )
kagome_install(binaryen_wasm_module)

add_library(binaryen_executor_factory
    core_api_factory_impl.cpp
    )
target_link_libraries(binaryen_executor_factory
    constant_code_provider
    binaryen_memory_provider
    core_api
    )
kagome_install(binaryen_executor_factory)

add_library(binaryen_instance_environment_factory
    instance_environment_factory.cpp
    )
target_link_libraries(binaryen_instance_environment_factory
    binaryen_executor_factory
    trie_storage_provider
    )
kagome_install(binaryen_instance_environment_factory)

add_library(binaryen_module_factory module/module_factory_impl.cpp)
target_link_libraries(binaryen_module_factory
    binaryen_wasm_module
    binaryen_instance_environment_factory
    )
kagome_install(binaryen_module_factory)
