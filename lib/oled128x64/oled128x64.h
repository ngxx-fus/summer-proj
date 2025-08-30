#ifndef __OLED128X64_H__
#define __OLED128X64_H__

#include "i2c_com.h"
#include "oled128x64_cmds.h"

#ifndef OLED128X64_ADDR
    #define OLED128X64_ADDR 0x3C
#endif 

/*
      0 1 2  y    127
    0 ###############
    1 ###############
    x #######p#######
    7 ###############

    For 3D buffer:
        p(x, y) <-> frame[pgID][clID][rowID]
    For 1D buffer:
        + Byte access:      __byte_at(x, y): (interget(x/8)*127 + y)
        + Read operation:   __read_at(x, y):  (__byte_at(x, y) & (1<<(x%8)))
        + Write 1-bit operation: __write_1_at(x, y):  (__byte_at(x, y) |= (1<<(x%8)))
        + Write 0-bit operation: __write_0_at(x, y):  (__byte_at(x, y) &= (~(1<<(x%8))))
        + Write op: __write_at(x, y, bit): ((bit)?__write_1_at(x, y):__write_0_at(x, y))

*/

#define __frame_buffer          oled_canvas
#define __byte_at(x, y)         __frame_buffer[(x/8)*127 + y]
#define __read_at(x, y)         (__byte_at(x, y) & (1<<(x%8)))
#define __write_1_at(x, y)      (__byte_at(x, y) |= (1<<(x%8)))
#define __write_0_at(x, y)      (__byte_at(x, y) &= (~(1<<(x%8))))
#define __write_at(x, y, bit)   ((bit)?__write_1_at(x, y):__write_0_at(x, y))

uint8_t oled_canvas[1024]= {0xFF,0xFF,0xFF,0x08,0xFF,0xFF,0x00,0xFF,
                            0x0, 0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};

void oled_init() {
    i2c_config_headers();
    uint8_t init_commands[] = {
        SSD1306_COMMAND,
        SSD1306_DISPLAY_OFF,
        SSD1306_SET_DISPLAY_CLOCK_DIV,      0x80,
        SSD1306_SET_MULTIPLEX_RATIO,        0x3F,
        SSD1306_SET_DISPLAY_OFFSET,         0x00,
        SSD1306_SET_START_LINE(0),
        SSD1306_CHARGE_PUMP,                0x14,
        SSD1306_SET_MEMORY_MODE,            0x00,
        SSD1306_SET_COM_PINS,               0x12,
        SSD1306_SET_CONTRAST,               0xCF,
        SSD1306_SET_PRECHARGE,              0xF1,
        SSD1306_SET_VCOM_DETECT,            0x40,
        SSD1306_ENTIRE_DISPLAY_ON
    };
    i2c_send_byte_array(OLED128X64_ADDR, init_commands, sizeof(init_commands)/sizeof(uint8_t));
}

void oled_turn_on_all_pixels(){
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_DISPLAY_ON);
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_ENTIRE_DISPLAY_ON);
}

void oled_show_RAM_content(){
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_DISPLAY_ON);
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_ENTIRE_DISPLAY_RESUME);
}

void oled_on(){
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_DISPLAY_ON);
}

void oled_off(){
    i2c_send_word(OLED128X64_ADDR, SSD1306_COMMAND, SSD1306_DISPLAY_OFF);
}

void oled_flush(){
    uint8_t cmds[] = {
        // SSD1306_SET_SEG_REMAP_127,
        // SSD1306_SET_COM_SCAN_DEC,
        // SSD1306_SET_COM_SCAN_DEC,   0x00,        // Memory mode = horizontal
        // SSD1306_SET_COLUMN_ADDR,    0x00, 0x7F,  // Column addr: 0 -> 127
        // SSD1306_SET_PAGE_ADDR,      0x00, 0x07,  // Page addr:   0 -> 7
        // SSD1306_NORMAL_DISPLAY,
        SSD1306_ENTIRE_DISPLAY_RESUME,
        SSD1306_DISPLAY_ON
    };
    i2c_send_byte_array_w_ctl_byte(OLED128X64_ADDR, SSD1306_COMMAND, cmds, sizeof(cmds));
    i2c_send_byte_array_w_ctl_byte(OLED128X64_ADDR, SSD1306_DATA, oled_canvas, sizeof(oled_canvas));
}

void oled_flush_area(uint8_t seg_start, uint8_t seg_end, uint8_t col_start, uint8_t col_end) {
    if (seg_end > 7) seg_end = 7;
    if (col_end > 127) col_end = 127;

    for (uint8_t page = seg_start; page <= seg_end; page++) {
        
        uint8_t cmd_page[3] = {
            (uint8_t)(0xB0 | page),
            (uint8_t)(0x00 | (col_start & 0x0F)),
            (uint8_t)(0x10 | (col_start >> 4))
        };
        i2c_send_byte_array_w_ctl_byte(OLED128X64_ADDR, SSD1306_COMMAND, cmd_page, 3);

        uint16_t offset = page * 128 + col_start;
        uint16_t len = col_end - col_start + 1;

        i2c_send_byte_array_w_ctl_byte(OLED128X64_ADDR, SSD1306_DATA, &oled_canvas[offset], len);
    }
}

#endif