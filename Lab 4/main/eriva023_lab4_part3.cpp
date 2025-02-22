
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"

//TODO: declare your global variables here
unsigned char First_push =0;
unsigned char Moves_made =0;
unsigned char correct_entries =0; 
bool unlocked_state = false;
bool center_position = true;
int combination[4] = {1, 2, 3, 4};  //store the combination in a array (in order of: Up, down, left, right)
unsigned char combination_index =0;
unsigned int i =0;
short motor_steps = 0;
bool flash_lights = false;
bool flash_lights_pt2 = false;
bool temp = false;
short flash_light_cnt =0;
bool unlocked_to_locked = false;
unsigned char counter_display =0;




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

enum states {INIT, display_zero, display_pushes, unlocked, incorrect, locked, change_password} state; //TODO: finish the enum for the SM

//NOTE: MAKE A LOCKED STATE 

void Tick() {
unsigned int adc_val_y = ADC_read(3);  // Read joystick y-axis
unsigned int adc_val_x = ADC_read(2);  // Read joystick x-axis

  // State Transistions
  //TODO: complete transitions 
  switch(state) {

    case INIT:
    state = display_zero;
    
    break;

    case display_zero:
    if(!(ADC_read(2) >= 500 && ADC_read(2) <= 620)){ // x-axis 
      state = display_pushes;
    }
    break;

    case display_pushes:
    if(correct_entries == 4 && unlocked_state == false){
      unlocked_state = true;
      state = unlocked;
      
    }
    if(flash_lights == true && unlocked_to_locked == false){
      state = incorrect;
    }
    
    break;

    case unlocked:
    //serial_println("STATE: unlocked");
   
     if(unlocked_to_locked == true){
      state = locked;
    }

    break;

    case incorrect:
    if(flash_lights == true && flash_light_cnt < 4000){
      if(flash_light_cnt % 500 == 0 && flash_lights_pt2 == false ){
        PORTC = SetBit(PORTC, 1, 0);  // Clear bit 1
        PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
        flash_lights_pt2 = true;
      }
      else if(flash_light_cnt % 500 == 0 && flash_lights_pt2 == true){
        PORTC = SetBit(PORTC, 1, 1);  // Clear bit 1
        PORTC = SetBit(PORTC, 0, 1);  // Clear bit 0
        flash_lights_pt2 = false;

      }
      flash_light_cnt++;
    }

    if(flash_light_cnt == 4000 && flash_lights == true){
      flash_light_cnt = 0; //reset variables before going back to display pushes 
      flash_lights = false;
      if(unlocked_state == false && temp == false){ //if its in that state and the user inputs the wrong combo, stay in it after
        PORTC = SetBit(PORTC, 1, 0);  // Clear bit 1
        PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
        state = display_pushes;
        

      }
      if(unlocked_to_locked == true && temp == true){
        PORTC = SetBit(PORTC, 1, 0);  // Clear bit 1
        PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
        state = locked;
      }
      
      
            
    }
    break;

    case locked:
    if(motor_steps < 1024 && correct_entries == 4){
       PORTB = (PORTB & 0x03) | phases[i] << 2;
      i--;
      if(i<0 || i>8){
        i = 7;
      }
      motor_steps++;
    }
    else if(motor_steps == 1024){
      motor_steps =0;
      correct_entries =0;
      temp = false;
      unlocked_to_locked = false;
      state = display_pushes;
    }

   
     if(flash_lights == true && unlocked_to_locked == true){
      state = incorrect;
    }

    if(GetBit(PINC, 4) == 0){
      PORTB = SetBit(PORTB, 1, 0b00000010 & 0x02);
      state = change_password;
    }


    

  
    break;

    case change_password:

    
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
    if ((adc_val_x >= 500 && adc_val_x <= 620) && (adc_val_y >= 500 && adc_val_y <= 620)) {
        outDir(5);  // Center position (neutral) 
        center_position = true;  // Prevent moves_made from incrementing repeatedly
    } 
    else if (adc_val_x > 580 && center_position) {
        outDir(1);  // Up
        if(combination[combination_index] == 1) {  // If the combination matches
          correct_entries += 1;
        }
        combination_index += 1;  // Move to the next combination index
        Moves_made += 1;  // Increment total moves made
        center_position = false;  // Reset center flag
    } 
    else if (adc_val_x < 500 && center_position) {
        outDir(2);  // Down
        if(combination[combination_index] == 2) {
          correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y < 500 && center_position) {
        outDir(3);  // Left
        if(combination[combination_index] == 3) {
          correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y > 585 && center_position) {
        outDir(4);  // Right
        if(combination[combination_index] == 4) {
           correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    }

    

    // Switch based on total moves made
    switch (Moves_made) {
        case 1:
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 2:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
            break;

        case 3:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 4:
            if(correct_entries == 4) {  // If all 4 moves are correct
                PORTC = SetBit(PORTC, 1, 1);
                PORTC = SetBit(PORTC, 0, 1);
                flash_lights = false;
            } 
            else if(correct_entries != 4 && Moves_made == 4){  // If the combination is incorrect
                PORTC = SetBit(PORTC, 1, 0);  // Clear bit 1
                PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
                Moves_made = 0;  // Reset moves made
                correct_entries =0;
                combination_index = 0;  // Reset combination index
                flash_lights = true;
                
            }
            break;

        default:
            break;
    }
    
    break;


    case unlocked:
    if(motor_steps < 1024 && correct_entries == 4 && unlocked_state == true){ // got to fix the if statements, make another bool variable most likely
      PORTB = (PORTB & 0x03) | phases[i] << 2;
      i++;
      if(i > 7){
        i = 0;
      }
      motor_steps++;
      
    }
    else if(motor_steps == 1024){
      //turn off leds after the stepper motor is done turning
      
      motor_steps =0; // reset variables
      correct_entries = 0;
      PORTC = SetBit(PORTC, 1, 0);
      PORTC = SetBit(PORTC, 0, 0);
      unlocked_state = false;
      flash_lights = false;

      temp = true;
      unlocked_to_locked = true; // After the stepper motor is done turning, move on to locked state with this variable 

    }
    
    
    break;

    case locked:
    if ((adc_val_x >= 500 && adc_val_x <= 620) && (adc_val_y >= 500 && adc_val_y <= 620)) {
        outDir(5);  // Center position (neutral) 
        center_position = true;  // Prevent moves_made from incrementing repeatedly
    } 
    else if (adc_val_x > 580 && center_position) {
        outDir(1);  // Up
        if(combination[combination_index] == 1) {  // If the combination matches
          correct_entries += 1;
        }
        combination_index += 1;  // Move to the next combination index
        Moves_made += 1;  // Increment total moves made
        center_position = false;  // Reset center flag
    } 
    else if (adc_val_x < 500 && center_position) {
        outDir(2);  // Down
        if(combination[combination_index] == 2) {
          correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y < 500 && center_position) {
        outDir(3);  // Left
        if(combination[combination_index] == 3) {
          correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y > 585 && center_position) {
        outDir(4);  // Right
        if(combination[combination_index] == 4) {
           correct_entries += 1;
        }
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    }

    

    // Switch based on total moves made
    switch (Moves_made) {
        case 1:
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 2:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
            break;

        case 3:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 4:
            if(correct_entries == 4) {  // If all 4 moves are correct
                PORTC = SetBit(PORTC, 1, 1);
                PORTC = SetBit(PORTC, 0, 1);
            } 
            else {  // If the combination is incorrect
                flash_lights = true;
                PORTC = SetBit(PORTC, 1, 0);  // Clear bit 1
                PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
                Moves_made = 0;  // Reset moves made
                correct_entries =0;
                combination_index = 0;  // Reset combination index
            }
            break;

        default:
            break;
    }
    break;


    case change_password:
     if ((adc_val_x >= 500 && adc_val_x <= 620) && (adc_val_y >= 500 && adc_val_y <= 620)) {
        outDir(5);  // Center position (neutral) 
        center_position = true;  // Prevent moves_made from incrementing repeatedly
    } 
    else if (adc_val_x > 580 && center_position ) { 
     
        outDir(1);  // Up
        combination[combination_index] = 1;
        combination_index += 1;  // Move to the next combination index
        Moves_made += 1;  // Increment total moves made
        center_position = false;  // Reset center flag
    } 
    else if (adc_val_x < 500 && center_position) {
        outDir(2);  // Down
        combination[combination_index] = 2;
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y < 500 && center_position) {
        outDir(3);  // Left
        combination[combination_index] = 3;
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    } 
    else if (adc_val_y > 585 && center_position) {
        outDir(4);  // Right
        combination[combination_index] = 4;
        combination_index += 1;
        Moves_made += 1;
        center_position = false;
    }

    

    // Switch based on total moves made
    switch (Moves_made) {
        case 1:
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 2:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 0);  // Clear bit 0
            break;

        case 3:
            PORTC = SetBit(PORTC, 1, 1);  // Set bit 1
            PORTC = SetBit(PORTC, 0, 1);  // Set bit 0
            break;

        case 4:
            if(Moves_made == 4) { 
              PORTC = SetBit(PORTC, 1, 0);  // turn off leds 
              PORTC = SetBit(PORTC, 0, 0);  // turn off leds 
              Moves_made = 0;  // Reset moves made
              correct_entries =0;
              combination_index = 0;  // Reset combination index
              PORTB &= ~(0b00000010); // Clear bit 1
              state = locked;
                
            } 
          
            break;

        default:
            break;
    }
    break;

    default:
      break;

      

  }

}



int main(void)
{
	//TODO: initialize all outputs and inputs


  ADC_init(); //initializes the analog to digital converters
	
  state = INIT;
  
  TimerSet(1); //period of 1 ms. good period for the stepper motor
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