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

#include <pico/binary_info.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"
#include "ssd1306.h"

static inline void swap(int32_t *a, int32_t *b) {
    int32_t *t = a;
    *a = *b;
    *b = *t;
}

#if DISP_PROTO == DISP_PROTO_SPI
static inline void cs_select(ssd1306_t *p) {
    asm volatile("nop \n nop \n nop");
    gpio_put(p->cs_pin, 0);
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect(ssd1306_t *p) {
    asm volatile("nop \n nop \n nop");
    gpio_put(p->cs_pin, 1);
    asm volatile("nop \n nop \n nop");
}
#endif

inline static void send_command(ssd1306_t *p, uint8_t command) {
#if DISP_PROTO == DISP_PROTO_I2C
    uint8_t d[2] = {0x00, command};
    i2c_write_blocking(p->i2c_i, p->address, d, 2, false);
#elif DISP_PROTO == DISP_PROTO_PIO_I2C
    uint8_t d[2] = {0x00, command};
    pio_i2c_write_blocking(p->pio, p->sm, p->address, d, 2);
#elif DISP_PROTO == DISP_PROTO_SPI
    gpio_put(p->dc_pin, 0);
    cs_select(p);
    spi_write_blocking(p->spi_i, &command, 1);
    cs_deselect(p);
#endif
}

inline static void send_commands(ssd1306_t *p, const uint8_t *commands,
                                 size_t len) {
#if DISP_PROTO == DISP_PROTO_SPI
    gpio_put(p->dc_pin, 0);
    cs_select(p);
    spi_write_blocking(p->spi_i, commands, len);
    cs_deselect(p);
#else
    for (size_t i = 0; i < len; ++i) {
        send_command(p, commands[i]);
    }
#endif
}

static bool ssd1306_init_common(ssd1306_t *p, uint16_t width, uint16_t height) {
    p->width = width;
    p->height = height;
    p->pages = height / 8;

    p->bufsize = (p->pages) * (p->width);
    if ((p->buffer = malloc(p->bufsize + 1)) == NULL) {
        p->bufsize = 0;
        return false;
    }

    ++(p->buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[] = {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV, 0x80, SET_MUX_RATIO, height - 1, SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP, p->external_vcc ? 0x10 : 0x14,
        SET_SEG_REMAP | 0x01,   // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08, // scan from COM[N] to COM0
        SET_COM_PIN_CFG, width > 2 * height ? 0x02 : 0x12,
        // display
        SET_CONTRAST, 0xff, SET_PRECHARGE, p->external_vcc ? 0x22 : 0xF1,
        SET_VCOM_DESEL,
        0x30,          // or 0x40?
        SET_ENTIRE_ON, // output follows RAM contents
        SET_NORM_INV,  // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00, // horizontal
    };

    send_commands(p, cmds, sizeof(cmds));

    return true;
}

#if DISP_PROTO == DISP_PROTO_I2C
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height,
                  uint8_t address, i2c_inst_t *i2c_instance) {
    p->address = address;
    p->i2c_i = i2c_instance;

    return ssd1306_init_common(p, width, height);
}
#elif DISP_PROTO == DISP_PROTO_PIO_I2C
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, PIO pio,
                  uint8_t address, uint8_t sda, uint8_t scl) {
    p->address = address;
    p->pio = pio;
    p->sda = sda;
    p->scl = scl;
    p->sm = 0;
    uint offset = pio_add_program(pio, &i2c_program);
    i2c_program_init(pio, p->sm, offset, sda, scl);

    return ssd1306_init_common(p, width, height);
}
#elif DISP_PROTO == DISP_PROTO_SPI
bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height,
                  spi_inst_t *spi_instance, uint8_t cs, uint8_t dc,
                  uint8_t rst) {
    p->spi_i = spi_instance;
    p->cs_pin = cs;
    p->dc_pin = dc;

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_put(cs, 1);

    gpio_init(dc);
    gpio_set_dir(dc, GPIO_OUT);

    gpio_init(rst);
    gpio_set_dir(rst, GPIO_OUT);

    // reset the display by pulling rst low then high
    gpio_put(rst, 0);
    sleep_ms(10);
    gpio_put(rst, 1);

    return ssd1306_init_common(p, width, height);
}
#endif

inline void ssd1306_deinit(ssd1306_t *p) { free(p->buffer - 1); }

inline void ssd1306_poweroff(ssd1306_t *p) { send_command(p, SET_DISP | 0x00); }

inline void ssd1306_poweron(ssd1306_t *p) { send_command(p, SET_DISP | 0x01); }

inline void ssd1306_contrast(ssd1306_t *p, uint8_t val) {
    uint8_t cmds[] = {SET_CONTRAST, val};
    send_commands(p, cmds, 2);
}

inline void ssd1306_invert(ssd1306_t *p, uint8_t inv) {
    send_command(p, SET_NORM_INV | (inv & 1));
}

inline void ssd1306_clear(ssd1306_t *p) { memset(p->buffer, 0, p->bufsize); }

void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    if (x >= p->width || y >= p->height) {
        return;
    }

    p->buffer[x + p->width * (y >> 3)] |=
        0x1 << (y & 0x07); // y>>3==y/8 && y&0x7==y%8
}

void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2,
                       int32_t y2) {
    if (x1 > x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    if (x1 == x2) {
        if (y1 > y2) {
            swap(&y1, &y2);
        }
        for (int32_t i = y1; i <= y2; ++i) {
            ssd1306_draw_pixel(p, x1, i);
        }
        return;
    }

    float m = (float)(y2 - y1) / (float)(x2 - x1);

    for (int32_t i = x1; i <= x2; ++i) {
        float y = m * (float)(i - x1) + (float)y1;
        ssd1306_draw_pixel(p, i, (uint32_t)y);
    }
}

void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width,
                         uint32_t height) {
    for (uint32_t i = 0; i < width; ++i) {
        for (uint32_t j = 0; j < height; ++j) {
            ssd1306_draw_pixel(p, x + i, y + j);
        }
    }
}

void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y,
                               uint32_t width, uint32_t height) {
    ssd1306_draw_line(p, x, y, x + width, y);
    ssd1306_draw_line(p, x, y + height, x + width, y + height);
    ssd1306_draw_line(p, x, y, x, y + height);
    ssd1306_draw_line(p, x + width, y, x + width, y + height);
}

void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y,
                                 uint32_t scale, const uint8_t *font, char c) {
    if (c<font[3] || c> font[4]) {
        return;
    }

    uint32_t parts_per_line = (font[0] >> 3) + ((font[0] & 7) > 0);
    for (uint8_t w = 0; w < font[1]; ++w) { // width
        uint32_t pp =
            (c - font[3]) * font[1] * parts_per_line + w * parts_per_line + 5;
        for (uint32_t lp = 0; lp < parts_per_line; ++lp) {
            uint8_t line = font[pp];

            for (int8_t j = 0; j < 8; ++j, line >>= 1) {
                if (line & 1) {
                    ssd1306_draw_square(p, x + w * scale,
                                        y + ((lp << 3) + j) * scale, scale,
                                        scale);
                }
            }

            ++pp;
        }
    }
}

void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y,
                                   uint32_t scale, const uint8_t *font,
                                   const char *s) {
    for (int32_t x_n = x; *s; x_n += (font[1] + font[2]) * scale) {
        ssd1306_draw_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}

void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale,
                       char c) {
    ssd1306_draw_char_with_font(p, x, y, scale, font_8x5, c);
}

void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale,
                         const char *s) {
    ssd1306_draw_string_with_font(p, x, y, scale, font_8x5, s);
}

static inline uint32_t ssd1306_bmp_get_val(const uint8_t *data,
                                           const size_t offset, uint8_t size) {
    switch (size) {
    case 1:
        return data[offset];
    case 2:
        return data[offset] | (data[offset + 1] << 8);
    case 4:
        return data[offset] | (data[offset + 1] << 8) |
               (data[offset + 2] << 16) | (data[offset + 3] << 24);
    default:
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data,
                                        const long size, uint32_t x_offset,
                                        uint32_t y_offset) {
    if (size < 54) { // data smaller than header
        return;
    }

    const uint32_t bfOffBits = ssd1306_bmp_get_val(data, 10, 4);
    const uint32_t biSize = ssd1306_bmp_get_val(data, 14, 4);
    const int32_t biWidth = (int32_t)ssd1306_bmp_get_val(data, 18, 4);
    const int32_t biHeight = (int32_t)ssd1306_bmp_get_val(data, 22, 4);
    const uint16_t biBitCount = (uint16_t)ssd1306_bmp_get_val(data, 28, 2);
    const uint32_t biCompression = ssd1306_bmp_get_val(data, 30, 4);

    if (biBitCount != 1) { // image not monochrome
        return;
    }

    if (biCompression != 0) { // image compressed
        return;
    }

    const int table_start = 14 + biSize;
    uint8_t color_val;

    for (uint8_t i = 0; i < 2; ++i) {
        if (!((data[table_start + i * 4] << 16) |
              (data[table_start + i * 4 + 1] << 8) |
              data[table_start + i * 4 + 2])) {
            color_val = i;
            break;
        }
    }

    uint32_t bytes_per_line = (biWidth / 8) + (biWidth & 7 ? 1 : 0);
    if (bytes_per_line & 3) {
        bytes_per_line = (bytes_per_line ^ (bytes_per_line & 3)) + 4;
    }

    const uint8_t *img_data = data + bfOffBits;

    int step = biHeight > 0 ? -1 : 1;
    int border = biHeight > 0 ? -1 : biHeight;
    for (uint32_t y = biHeight > 0 ? biHeight - 1 : 0; y != border; y += step) {
        for (uint32_t x = 0; x < biWidth; ++x) {
            if (((img_data[x >> 3] >> (7 - (x & 7))) & 1) == color_val) {
                ssd1306_draw_pixel(p, x_offset + x, y_offset + y);
            }
        }
        img_data += bytes_per_line;
    }
}

inline void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data,
                                   const long size) {
    ssd1306_bmp_show_image_with_offset(p, data, size, 0, 0);
}

void ssd1306_show(ssd1306_t *p) {
    uint8_t payload[] = {SET_COL_ADDR,  0, p->width - 1,
                         SET_PAGE_ADDR, 0, p->pages - 1};
    if (p->width == 64) {
        payload[1] += 32;
        payload[2] += 32;
    }

    send_commands(p, payload, sizeof(payload));

    *(p->buffer - 1) = 0x40;

#if DISP_PROTO == DISP_PROTO_I2C
    i2c_write_blocking(p->i2c_i, p->address, p->buffer - 1, p->bufsize + 1,
                       false);
#elif DISP_PROTO == DISP_PROTO_PIO_I2C
    pio_i2c_write_blocking(p->pio, p->sm, p->address,
                           p->buffer - 1, p->bufsize + 1);
#elif DISP_PROTO == DISP_PROTO_SPI
    gpio_put(p->dc_pin,
             1); // bring data/command high since we're sending data now
    cs_select(p);
    spi_write_blocking(p->spi_i, p->buffer, p->bufsize);
    cs_deselect(p);
#endif
}