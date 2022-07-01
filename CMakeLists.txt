add_library(ssd1306_spi INTERFACE)
target_sources(ssd1306_spi INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306_spi.c
)
target_include_directories(ssd1306_spi INTERFACE
    include
)
target_link_libraries(ssd1306_spi INTERFACE
    pico_stdlib
    hardware_spi
)

