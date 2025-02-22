#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <timerISR.h>



volatile unsigned char TimerFlag =0; // ISR raises, main() lowers

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

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 7SEG layout:   abcdefg
int nums[17] = {0b1111110, // 0
                0b0110000, // 1
                0b1101101, // 2
                0b1111001, // 3
                0b0110011, // 4
                0b1011011, // 5
                0b1011111, // 6
                0b1110000, // 7
                0b1111111, // 8
                0b1111011, // 9
                0b1110111, // a
                0b0011111, // b
                0b1001110, // c
                0b0111101, // d
                0b1001111, // e
                0b1000111, // f
                0b0000000, // off
                }; 

//TODO: complete outNum()

void outNum(int num){
	PORTB = nums[num] >> 1;/*depends on your wiring*/ //assigns bits 1-7 of nums(a-f)
  PORTD = SetBit(PORTD, 7 ,nums[num]&0x01); // assigns bit 0 of nums(g)
}

enum states {INIT, off, on, on_release, transiton, timer } state; //TODO: finish the enum for the SM
long input_min = 0;
long input_max = 1023;
long output_min = 0;
long output_max =16;

long input_min_photoR = 0;
long input_max_photoR = 100;
long output_min_photoR = 0;
long output_max_photoR = 1000;
long i;





void Tick() {

  // State Transistions
  //TODO: complete transitions 
  switch(state) {

    case INIT:
    state = off;
    break;

    case off:
     //serial_println("In State_off");
    if(GetBit(PINC, 5) != 0){     //if button on pin A5 is pressed, go to the ON STATE
    state = on;
    } 
    else if(GetBit(PINC, 5) == 0){ // If button is not pressed stay off
    state = off;
    }
      break;

    case on:
    //serial_println("In State_on");
    
    if(GetBit(PINC, 5) != 0){ // As long as the button is pressed stay in the on state, this will happen quick
      state = on;             // it'll go to the next as soon as its released 
    }
    else if(GetBit(PINC, 5) == 0){ // released, immeidately go to the on_release state 
      state = on_release;
    }
    break;

    case on_release:

    //serial_println("In State_on_release"); 
    //serial_println(map(ADC_read(1), input_min, input_max, output_min_photoR, output_max_photoR));  ///////////////////////////////////////////////////////////////////////////////
    if(GetBit(PINC, 5) != 0 && map(ADC_read(1), input_min, input_max, output_min_photoR, output_max_photoR) < 27){      //If the button is pressed, go to the off_release state
      state = transiton; 
      //PORTD = SetBit(PORTD,3,1);
      //PORTD = SetBit(PORTD,4,1);
    }
    else if(GetBit(PINC, 5) == 0){ // otherwise stay in this state
      state = on_release;

    }
    break;

    case transiton:
    //serial_println("In State_transition");
    i = map(ADC_read(0), input_min, input_max, output_min, output_max);
    //outNum(map(ADC_read(0), input_min, input_max,  output_min, output_max));
    
    if(GetBit(PINC, 5) != 0){ //If the button is pressed, stay
      state = transiton;
    }
    else if(GetBit(PINC, 5) == 0){
      state = timer;
    }
    break;

    case timer:
    PORTD = SetBit(PORTD,3,1); // RGB  red on
    PORTD = SetBit(PORTD,2,0); // RGB LED green off
    PORTD = SetBit(PORTD,4,1);
    if(GetBit(PINC, 5) != 0 || map(ADC_read(1), input_min, input_max, output_min_photoR, output_max_photoR) > 27 ){
      state = off;
    }
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

    case off:
    PORTD = SetBit(PORTD,2,1); // LED stays Green
    PORTD = SetBit(PORTD,4,0); //reg led
    PORTD = SetBit(PORTD,3,0);
  
    outNum(16);
    break;

    case on:
    PORTD = SetBit(PORTD,2,1); //LED strys Green
    break;

    case on_release:
    outNum(map(ADC_read(0), input_min, input_max,  output_min, output_max)); //turn on display, and show different values when moving POT
    break;

    case transiton:
    
    break;

    case timer:
     if(i >= 0) {
            //serial_println(i);
            outNum(i);           // Output current value of i to 7-segment display
            i--;                 // Decrement i every 1 second
        } else {
          PORTD = SetBit(PORTD,2,1); // LED stays Green
          PORTD = SetBit(PORTD,4,0); //reg led
          PORTD = SetBit(PORTD,3,0);
            outNum(16);
            state = off;        // Transition to next state when countdown is complete
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
   

    TimerSet(1000);  //TODO: Set your timer
    TimerOn();

    DDRB = 0xFF; PORTB = 0x00; //Turns all of the B ports as outputs (to control an output,  make one of the ports/bits into 1)
    DDRD = 0xFF; PORTD = 0x00; 
    DDRC = 0x00; PORTC = 0xFF;

    while (1)
    {

	    Tick();      // Execute one synchSM tick
        while (!TimerFlag){}  // Wait for SM period
        TimerFlag = 0;        // Lower flag
    }

    return 0;
}