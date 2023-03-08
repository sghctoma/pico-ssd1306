/*
MIT License

Copyright (c) 2021 David Schramm
Copyright (c) 2023 Tamás Szakály (sghctoma)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _SSD1306_SPI_H
#define _SSD1306_SPI_H

#include <hardware/spi.h>

/**
 * @brief holds the configuration
 */
typedef struct {
    uint8_t width;  /**< width of display */
    uint8_t height; /**< height of display */
    bool flipped;   /**< whether the display is flipped vertically */
    uint8_t pages;  /**< stores pages of display (calculated on initialization*/
    spi_inst_t *spi_i; /**< spi connection instance */
    uint8_t cs_pin;    /**< chip select pin */
    uint8_t dc_pin;    /**< data/command pin */
    bool external_vcc; /**< whether display uses external vcc */
    uint8_t *buffer;   /**< display buffer */
    size_t bufsize;    /**< buffer size */
} ssd1306_t;

/**
 * @brief initialize display. It will initialize CS, DC and RST pins (those are
 *        unique for each connected peripheral), but it won't initialize SPI (
 *        including MOSI and SCK pin setup), because the same SPI could also be
 *        used by other peripherals.
 *
 * @param[in] p : pointer to instance of ssd1306_t
 * @param[in] width : width of display
 * @param[in] height : heigth of display
 * @param[in] spi_instance : instance of SPI connection
 * @param[in] cs : chip select PIN
 * @param[in] dc : data/command PIN
 * @param[in] rst : reset PIN
 *
 * @return bool.
 * @retval true for Success
 * @retval false if initialization failed
 */
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height,
                  spi_inst_t *spi_instance, uint8_t cs, uint8_t dc,
                  uint8_t rst);

#endif // _SSD1306_SPI_H