add_library(ssd1306_i2c INTERFACE)
target_sources(ssd1306_i2c INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
)
target_include_directories(ssd1306_i2c INTERFACE
    include
)
target_link_libraries(ssd1306_i2c INTERFACE
    pico_stdlib
    hardware_i2c
)
