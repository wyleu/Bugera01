/* riban modular
  
  Copyright 2023 riban ltd <info@riban.co.uk>

  This file is part of riban modular.
  riban modular is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
  riban modular is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
  You should have received a copy of the GNU General Public License along with riban modular. If not, see <https://www.gnu.org/licenses/>.

  Impelentation of WS2812 device driver (ab)using  Roger Clark's SPI interface library
  Based on https://www.newinnovations.nl/post/controlling-ws2812-and-ws2812b-using-only-stm32-spi
*/

#include "ws2812.h"

uint8_t * ws2812_buffer; // Pointer to buffer holding data to send to WS2812 via SPI 
uint16_t ws2812_num_leds = 0; // Quantity of LEDs controlled
bool ws2812_pending = false; // True indicates a value has changed since last refresh
SPIClass * ws2812_spi; // Pointer to the SPI interface

void ws2812_init(uint8_t spi_port, uint16_t leds) {
    ws2812_num_leds = leds;
    ws2812_buffer = new uint8_t[WS2812_BUFFER_SIZE];
    SPISettings spiSettings(4000000, MSBFIRST, SPI_MODE_3);
    ws2812_spi = new SPIClass(spi_port);
    ws2812_spi->beginTransaction(spiSettings);
    memset(ws2812_buffer, 0, WS2812_BUFFER_SIZE);
    ws2812_set_all(0, 0, 0);
    ws2812_refresh();
}

void ws2812_refresh(bool mute) {
    if (mute) {
        for(uint16_t i = 0; i < ws2812_num_leds * 24; ++i)
            ws2812_spi->transfer(0b10000000);
        for(uint16_t i = 0; i < WS2812_RESET_PULSE; ++i)
            ws2812_spi->transfer(0b00000000);
    } else if (ws2812_pending) {
        ws2812_spi->write(ws2812_buffer, WS2812_BUFFER_SIZE);
        ws2812_pending = false;
    }
}

#define WS2812_FILL_BUFFER(COLOUR) \
    for( uint8_t mask = 0x80; mask; mask >>= 1 ) { \
        if( COLOUR & mask ) { \
            *ptr++ = 0b11111100; \
        } else { \
            *ptr++ = 0b10000000; \
        } \
    }

void ws2812_set(uint16_t led, uint8_t red, uint8_t green, uint8_t blue) {
    if (led >= ws2812_num_leds)
        return;
    uint8_t * ptr = &ws2812_buffer[24 * led];
    WS2812_FILL_BUFFER(green);
    WS2812_FILL_BUFFER(red);
    WS2812_FILL_BUFFER(blue);
    ws2812_pending = true;
}

void ws2812_set_all(uint8_t red, uint8_t green, uint8_t blue) {
    uint8_t * ptr = ws2812_buffer;
    for( uint16_t i = 0; i < ws2812_num_leds; ++i) {
        WS2812_FILL_BUFFER(green);
        WS2812_FILL_BUFFER(red);
        WS2812_FILL_BUFFER(blue);
    }
}
