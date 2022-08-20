if(PICO_CYW43_SUPPORTED)
    message("Watchdog - PicoW")
    add_definitions(-DPICO_WIFI)

    add_executable(watchdog
        ../third_party/pico-lora/src/LoRa-RP2040.h
        ../third_party/pico-lora/src/LoRa-RP2040.cpp
        ../third_party/pico-lora/src/Print.h
        ../third_party/pico-lora/src/Print.cpp
        ../third_party/tiny-AES-c/aes.h
        ../third_party/tiny-AES-c/aes.hpp
        ../third_party/tiny-AES-c/aes.c
        ../common/common.h
        ../common/common.cpp
        lwipopts.h
        watchdog-config.h
        dateutils.h
        dateutils.cpp
        poller.h
        poller.cpp
        dns.h
        dns.cpp
        ntp.h
        ntp.cpp
        tcpclient.h
        tcpclient.cpp
        main.cpp
    )

    target_include_directories(watchdog PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    # NOTE: currenty I'm using my pico-sdk fork: https://github.com/tomicooler/pico-sdk/commit/cefa36599c5734fc84ffccfe75869680f1b727d1
    # TODO: update this when this PR is merged: https://github.com/raspberrypi/pico-sdk/pull/894
    target_link_libraries(watchdog pico_stdlib hardware_spi hardware_interp pico_cyw43_arch_lwip_threadsafe_background pico_stdlib pico_lwip_mbedtls pico_mbedtls)
else()
    message("Watchdog - Pico")
    add_executable(watchdog
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

    target_link_libraries(watchdog pico_stdlib hardware_spi hardware_interp)
endif()

# enable usb output, disable uart output
pico_enable_stdio_usb(watchdog 1)
pico_enable_stdio_uart(watchdog 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(watchdog)
