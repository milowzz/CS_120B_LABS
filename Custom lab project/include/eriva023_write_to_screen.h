#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eriva023_serialATmega.h"
#include "eriva023_spi.h"

 const uint8_t font[42][5] = {
    { 0x7C, 0x12, 0x11, 0x12, 0x7C }, // 'A' (No change)
    { 0x6C, 0x91, 0x91, 0x91, 0xFE }, // 'B' (No change)
    { 0x22, 0x41, 0x41, 0x41, 0x3E }, // 'C' (Mirrored)
    { 0x1C, 0x22, 0x41, 0x41, 0x7F }, // 'D' (Mirrored)
    { 0x41, 0x49, 0x49, 0x49, 0x7F }, // 'E' (No change)
    { 0x01, 0x09, 0x09, 0x09, 0x7F }, // 'F' (No change)
    { 0x7A, 0x49, 0x49, 0x41, 0x3E }, // 'G' (Mirrored)
    { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // 'H' (No change)
    { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // 'I' (No change)
    { 0x01, 0x3F, 0x41, 0x40, 0x20 }, // 'J' (Mirrored)
    { 0x41, 0x22, 0x14, 0x08, 0x7F }, // 'K' (Mirrored)
    { 0x40, 0x40, 0x40, 0x40, 0x7F }, // 'L' (No change)
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // 'M' (No change)
    { 0x7F, 0x10, 0x08, 0x04, 0x7F }, // 'N' (Mirrored)
    { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // 'O' (No change)
    { 0x06, 0x09, 0x09, 0x09, 0x7F }, // 'P' (Mirrored)
    { 0x5E, 0x21, 0x51, 0x41, 0x3E }, // 'Q' (Mirrored)
    { 0x46, 0x29, 0x19, 0x09, 0x7F }, // 'R' (Mirrored)
    { 0x31, 0x49, 0x49, 0x49, 0x46 }, // 'S' (Mirrored)
    { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // 'T' (No change)
    { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // 'U' (No change)
    { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // 'V' (No change)
    { 0x7F, 0x20, 0x18, 0x20, 0x7F }, // 'W' (No change)
    { 0x63, 0x14, 0x08, 0x14, 0x63 }, // 'X' (No change)
    { 0x03, 0x04, 0x78, 0x04, 0x03 }, // 'Y' (No change)
    { 0x43, 0x45, 0x49, 0x51, 0x61 }, // 'Z' (Mirrored)
    { 0x00, 0x14, 0x14, 0x00, 0x00 }, // ':'
    { 0x00, 0x00, 0x00, 0x00, 0x00 }, // ' ' 
    { 0x00, 0x07, 0x00, 0x07, 0x00 }, // '"'
    { 0x14, 0x7F, 0x14, 0x7F, 0x14 }, // '#'
    { 0x24, 0x2A, 0x7F, 0x2A, 0x12 }, // '$'
    { 0x23, 0x13, 0x08, 0x64, 0x62 }, // '%'
    { 0x36, 0x49, 0x55, 0x22, 0x50 }, // '&'
    { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '''
    { 0x00, 0x1C, 0x22, 0x41, 0x00 }, // '('
    { 0x00, 0x41, 0x22, 0x1C, 0x00 }, // ')'
    { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // '*'
    { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // '+'
    { 0x00, 0x50, 0x30, 0x00, 0x00 }, // ','
    { 0x08, 0x08, 0x08, 0x08, 0x08 }, // '-'
    { 0x00, 0x60, 0x60, 0x00, 0x00 }, // '.'
    { 0x20, 0x10, 0x08, 0x04, 0x02 }, // '/'

};


void DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    // Set column address (CASET)
    Send_Command(0x2A);  // CASET
    Send_data(0x00);     // Start column high byte
    Send_data(x & 0xFF); // Start column low byte
    Send_data(0x00);     // End column high byte
    Send_data(x & 0xFF); // End column low byte

    // Set row address (RASET)
    Send_Command(0x2B);  // RASET
    Send_data(0x00);     // Start row high byte
    Send_data(y & 0xFF); // Start row low byte
    Send_data(0x00);     // End row high byte
    Send_data(y & 0xFF); // End row low byte

    // Write pixel data (RAMWR)
    Send_Command(0x2C);  // RAMWR
    Send_data((color >> 8) & 0xFF); // Send high byte of color
    Send_data(color & 0xFF);        // Send low byte of color
}


void DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg) {
    for (uint8_t i = 0; i < 5; i++) { // Character width is 5 pixels
        uint8_t line = font[c - 'A'][i]; // Get the font data for the character

        for (uint8_t j = 0; j < 8; j++) { // Character height is 8 pixels
            if (line & (1 << (7 - j))) { // Check each bit from MSB to LSB
                DrawPixel(x + i, y + j, color); // Correct vertical position
            } else {
                DrawPixel(x + i, y + j, bg); // Correct vertical position
            }
        }
    }
}

void DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg) {
    while (*str) {
        DrawChar(x, y, *str++, color, bg);
        x -= 6; // Advance x position (5 pixels + 1 pixel spacing)
    }
}

void display_screen_two(){
     DrawString(122,120, "PLAYER TWO PICK",0xFFFF, 0x0000);
}

void clear_screen(){
    //DrawString(122,120, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    //DrawString(122,110, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,100, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,90, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,80, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,70, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,60, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,50, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,40, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    //DrawString(122,30, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    //DrawString(122,20, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    //DrawString(122,10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
}

void clear_whole_screen(){
    DrawString(122,120, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,110, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,100, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,90, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,80, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,70, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,60, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,50, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,40, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,30, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,20, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);

}

void turn_gameOff(){

    SetCSToHigh_display_two(); // turn off communication for display two
    SetCSToHigh_display_one(); // turn off communication for display two


    SetCSToLow_display_one();  // turn on communication for display one
    SetCSToHigh_display_two(); // turn off communication for display two
    
    DrawString(122,120, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,110, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,100, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,90, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,80, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,70, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,60, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,50, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,40, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,30, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,20, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);

    SetCSToHigh_display_one();  // turn on communication for display one
    SetCSToLow_display_two(); // turn off communication for display two
    DrawString(122,120, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,110, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,100, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,90, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,80, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,70, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,60, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,50, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,40, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,30, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,20, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
    DrawString(122,10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);

}

