add_executable(doorwatcher
    ../third_party/pico-lora/src/LoRa-RP2040.h
    ../third_party/pico-lora/src/LoRa-RP2040.cpp
    ../third_party/pico-lora/src/Print.h
    ../third_party/pico-lora/src/Print.cpp
    ../third_party/tiny-AES-c/aes.h
    ../third_party/tiny-AES-c/aes.hpp
    ../third_party/tiny-AES-c/aes.c
    ../common/common.h
    ../common/common.cpp
    main.cpp
)

# pull in common dependencies
target_link_libraries(doorwatcher pico_stdlib hardware_spi hardware_interp hardware_sleep)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(doorwatcher)

# enable usb output, disable uart output
pico_enable_stdio_usb(doorwatcher 0)
pico_enable_stdio_uart(doorwatcher 1)
