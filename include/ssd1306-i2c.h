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

#ifndef _SSD1306_I2C_H
#define _SSD1306_I2C_H

#include <hardware/i2c.h>

/**
 * @brief holds the configuration
 */
typedef struct {
    uint8_t width;  /**< width of display */
    uint8_t height; /**< height of display */
    uint8_t pages;  /**< stores pages of display (calculated on initialization*/
    uint8_t address;   /**< i2c address of display*/
    i2c_inst_t *i2c_i; /**< i2c connection instance */
    bool external_vcc; /**< whether display uses external vcc */
    uint8_t *buffer;   /**< display buffer */
    size_t bufsize;    /**< buffer size */
} ssd1306_t;

/**
 * @brief initialize display
 *
 * @param[in] p : pointer to instance of ssd1306_t
 * @param[in] width : width of display
 * @param[in] height : heigth of display
 * @param[in] address : i2c address of display
 * @param[in] i2c_instance : instance of i2c connection
 *
 * @return bool.
 * @retval true for Success
 * @retval false if initialization failed
 */
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height,
                  uint8_t address, i2c_inst_t *i2c_instance);

#endif // _SSD1306_I2C_H