
add_library(validator_parachain
    availability/store/store_impl.cpp
    validator/impl/parachain_observer.cpp
    validator/impl/parachain_processor.cpp
    )

target_link_libraries(validator_parachain
    req_collation_protocol
    collation_protocol
    protocol_error
    )
