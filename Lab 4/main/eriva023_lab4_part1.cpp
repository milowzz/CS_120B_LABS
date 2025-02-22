#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"


//TODO: declare your global variables here
unsigned char First_push =0;
unsigned char button_pressed =0;

volatile unsigned char TimerFlag =0; // ISR raises, main() lowers

//NOTE: USE THIS ONCE CODE IS RREADY!!!! unsigned char passcode[4] = {} //TODO: for exercise 2 and 3, the initial passcode should be up, down, left, right
void TimerISR() {
   TimerFlag = 1;
}

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

void ADC_init() {
  ADMUX = (1<<REFS0);
	ADCSRA|= (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}



unsigned int ADC_read(unsigned char chnl){
  uint8_t low, high;

  ADMUX  = (ADMUX & 0xF8) | (chnl & 7);
  ADCSRA |= 1 << ADSC ;
  while((ADCSRA >> ADSC)&0x01){}


	low  = ADCL;
	high = ADCH;

	return ((high << 8) | low) ;
}




//directions[] and outDir() replaces nums[] and outNum() from previous exercise.
//they behave the same, the only difference is outDir() outputs 4 direction and a neutral
int directions[6] = { 
  0b1111110, // 0 (zero)
  0b0111110, // U (up)
  0b0111101, // d (down)
  0b0001110, // L (left)
  0b0000101, // r (right)
  0b0000001, // - (center)

}; //TODO: complete the array containg the values needed for the 7 segments for each of the 4 directions
// a  b  c  d  e  f  g
//TODO: display the direction to the 7-seg display. HINT: will be very similar to outNum()
void outDir(int dir){
  PORTD = directions[dir] << 1; //assigns bits 1-7 of nums(a-f) to pins 2-7 of port d
  PORTB = SetBit(PORTB, 0 ,directions[dir]&0x01); // assigns bit 0 of nums(g) to pin 0 of port b

}

int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //8 phases of the stepper motor step

enum states {INIT, display_zero, display_pushes } state; //TODO: finish the enum for the SM


void Tick() {

  // State Transistions
  //TODO: complete transitions 
  switch(state) {

    case INIT:
    state = display_zero;
    
    break;

    case display_zero:
    if(!(ADC_read(2) >= 558 && ADC_read(2) <= 565)){ 
      state = display_pushes;
      First_push += 1; // detect when the user moves the Joystick once to leave this state
    }
    
    break;

    case display_pushes:
    

    break;


    

    default:
      state = INIT;
      break;

  }

  // State Actions
  //TODO: complete transitions
  switch(state) {

    case INIT:
      break;

    case display_zero:
    outDir(0); //display zero
    ADC_read(2); // A2 (2) checks the x axis 
    break;

    case display_pushes:
    unsigned int adc_val_x = ADC_read(2);  // Read joystick x-axis
    unsigned int adc_val_y = ADC_read(3);  // Read joystick y-axis

    // Delay for readability (1 second delay)
    //_delay_ms(1000);
    

    // Check the direction and update the 7-segment display accordingly
    if (First_push == 1) {
        if (adc_val_x >= 558 && adc_val_x <= 565 && adc_val_y >= 573 && adc_val_y <= 578) {
            outDir(5);  // Center position (neutral)
        } else if (adc_val_x > 580) {
            outDir(1);  // Up
        } else if (adc_val_x < 550) {
            outDir(2);  // Down
        } else if (adc_val_y < 560) {
            outDir(3);  // Left
        } else if (adc_val_y > 585) {
            outDir(4);  // Right
        }

        if(GetBit(PINC,4) == 0){
          button_pressed += 1;
        }

        switch (button_pressed)
        {
          _delay_ms(400);
        case 1:
        PORTC = SetBit(PORTC,0,1);
        break;
        case 2:
        PORTC = SetBit(PORTC,1,1);
        PORTC = SetBit(PORTC,0,0);
        break;
        case 3:
        PORTC = SetBit(PORTC,1,1);
        PORTC = SetBit(PORTC,0,1);
        break;
        case 4:
        PORTC = SetBit(PORTC,1,0);
        PORTC = SetBit(PORTC,0,0);
        button_pressed = 0;
        break;

        
        default:
          break;
        }

    }
    break;

    default:
      break;

  }

}



int main(void)
{
	//TODO: initialize all outputs and inputs


  ADC_init();//initializes the analog to digital converter
	
  state = INIT;

  TimerSet(100); //period of 1 ms. good period for the stepper mottor
  TimerOn();

  DDRB = 0xFF; PORTB = 0x00; //Turns all of the B ports as outputs (to control an output,  make one of the ports/bits into 1)
  DDRD = 0xFF; PORTD = 0x00; // PORTD are outputs 
  DDRC = 0x03; PORTC = 0xFC; // make bits 0x7C as inputs, subtra
  


    while (1)
    {

		  Tick();      // Execute one synchSM tick
      while (!TimerFlag){}  // Wait for SM period
      TimerFlag = 0;        // Lower flag
     }

    return 0;
}