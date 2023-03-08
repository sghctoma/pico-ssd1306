add_library(ssd1306 INTERFACE)

set(SSD1306_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
)

SET(DISP_PROTO I2C CACHE STRING "Display connection protocol (I2C, SPI or PIO_I2C)")

if(${DISP_PROTO} STREQUAL I2C)
    set(PROTO_LIB hardware_i2c)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=1)
elseif(${DISP_PROTO} STREQUAL SPI)
    set(PROTO_LIB hardware_spi)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=2)
elseif(${DISP_PROTO} STREQUAL PIO_I2C)
    set(PROTO_LIB hardware_pio)
    target_compile_options(ssd1306 INTERFACE -DDISP_PROTO=3)
    set(SSD1306_SOURCES
        ${SSD1306_SOURCES}
        ${CMAKE_CURRENT_LIST_DIR}/pio_i2c.c
    )
    pico_generate_pio_header(ssd1306
        ${CMAKE_CURRENT_LIST_DIR}/i2c.pio
    )
endif()

target_sources(ssd1306 INTERFACE
    ${SSD1306_SOURCES}
)

target_include_directories(ssd1306 INTERFACE
    include
)
target_link_libraries(ssd1306 INTERFACE
    pico_stdlib
    ${PROTO_LIB}
)

