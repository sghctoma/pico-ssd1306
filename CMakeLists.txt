add_library(ssd1306 INTERFACE)

SET(DISP_PROTO PIO_I2C CACHE STRING "Display connection protocol (I2C, SPI or PIO_I2C)")

if(${DISP_PROTO} STREQUAL I2C)
    message("Building SSD1306 driver with I2C protocol.")
    set(PROTO_LIB hardware_i2c)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=1)
elseif(${DISP_PROTO} STREQUAL SPI)
    message("Building SSD1306 driver with SPI protocol.")
    set(PROTO_LIB hardware_spi)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=2)
elseif(${DISP_PROTO} STREQUAL PIO_I2C)
    message("Building SSD1306 driver with I2C via PIO protocol.")
    set(PROTO_LIB hardware_pio)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=3)
endif()

target_sources(ssd1306 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
)

target_include_directories(ssd1306 INTERFACE
    include
)
target_link_libraries(ssd1306 INTERFACE
    pico_stdlib
    ${PROTO_LIB}
)

