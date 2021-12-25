add_executable(doorwatcher
        main.cpp
        )

# pull in common dependencies
target_link_libraries(doorwatcher pico_stdlib)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(doorwatcher)
