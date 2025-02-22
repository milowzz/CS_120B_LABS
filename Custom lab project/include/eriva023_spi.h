#ifndef SPIAVR_H
#define SPIAVR_H
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "eriva023_helper.h"

/*B5 should always be SCK(spi clock) and B3 should always be MOSI(SDA). If you are using
an SPI peripheral that sends data back to the arduino, you will need to use B4 as the MISO pin. 
The SS pin can be any digital pin on the arduino. Right before sending an 8 bit
value with the SPI_SEND() funtion, you will need to set your SS(CS)pin to low. If you have
multiple SPI devices, they will share the SCK, MOSI and MISO pins but should have different SS
pins. To send a value to a specific device, set it's SS pin to low and all other SS
pins to high. Outputs, pin definitions */

#define PIN_SCK PORTB5//SHOULD ALWAYS BE B5 ON THE ARDUINO
#define PIN_MOSI PORTB3//SHOULD ALWAYS BE B3 ON THE ARDUINO
#define PIN_SS PORTB2
#define PIN_SS_2 PORTD3

//If SS is on a different port, make sure to change the init to take that into account.
void SPI_INIT(){
DDRB |= (1 << PIN_SCK) | (1 << PIN_MOSI) | (1 << PIN_SS);//initialize your pins.
DDRD |= (1 << PIN_SS_2); //initialize pin for second display
SPCR |= (1 << SPE) | (1 << MSTR); //initialize SPI coomunication
}
void SPI_SEND(char data)
{
SPDR = data;//set data that you want to transmit
while (!(SPSR & (1 << SPIF)));// wait until done transmitting
}

void SetCSToLow_display_one(){ 
  PORTB = SetBit(PORTB, 2, 0); // cs to low,
}

void SetCSToLow_display_two(){ 
  PORTD = SetBit(PORTD, 3, 0); // cs to low,
}

void SetCSToHigh_display_one(){
    PORTB = SetBit(PORTB, 2, 1); // cs to high, ends communication
}

void SetCSToHigh_display_two(){
    PORTD = SetBit(PORTD, 3, 1); // cs to high, ends communication for display two
}

void SetA0ToLow_command(){
   PORTD = SetBit(PORTD, 5, 0); //pin 5/ D5
}

void SetA0ToHigh_data(){
    PORTD = SetBit(PORTD, 5, 1); //pin 5/ D5
}


void Send_Command(char command) {
    //SetCSToLow();    // CS = 0: Start communication
    SetA0ToLow_command();    // A0 = 0: Indicate command
    SPI_SEND(command); // Send the command via SPI
    //SetCSToHigh();   // CS = 1: End communication
}

void Send_data(char command) {
    //SetCSToLow();    // CS = 0: Start communication
    SetA0ToHigh_data();    // A0 = 1: Indicate data
    SPI_SEND(command); // Send the command via SPI
    //SetCSToHigh();   // CS = 1: End communication
}

 
        


#endif /* SPIAVR_H */
