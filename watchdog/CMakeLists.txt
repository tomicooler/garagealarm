add_executable(watchdog
    ../third_party/pico-lora/src/LoRa-RP2040.h
    ../third_party/pico-lora/src/LoRa-RP2040.cpp
    ../third_party/pico-lora/src/Print.h
    ../third_party/pico-lora/src/Print.cpp
    main.cpp
)

# pull in common dependencies
target_link_libraries(watchdog pico_stdlib hardware_spi hardware_interp)

# enable usb output, disable uart output
pico_enable_stdio_usb(watchdog 1)
pico_enable_stdio_uart(watchdog 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(watchdog)
