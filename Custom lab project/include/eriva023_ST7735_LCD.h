#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "eriva023_spi.h"
#include "eriva023_helper.h"

void HardwareReset(){
PORTB = SetBit(PORTB, 0, 0); //reset pin B0, aka pin 8
_delay_ms(200);
PORTB = SetBit(PORTB, 0, 1);
_delay_ms(200);
}

void ST7735_init(){
HardwareReset();
Send_Command(0x01); // software reset
_delay_ms(150);
Send_Command(0x11); // Sleep out & booster on
_delay_ms(200);
Send_Command(0x3A); // interface pixel format
Send_data(0x05);
_delay_ms(10);
Send_Command(0x29); // Display On
_delay_ms(200);
}
