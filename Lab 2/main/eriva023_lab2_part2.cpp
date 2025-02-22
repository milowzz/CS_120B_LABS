/*         Name: Emilio Rivas
           Email: eriva023@ucr.edu

*          Discussion Section: 024

 *         Assignment: Lab #2 Exercise #2

 *         Exercise Description: [optional - include for your own benefit]

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *         Demo Link: <>

 */


#include <avr/io.h>
#include <stdio.h>


 //this will counter the amount of iterations the state machine does until it needs to reset again
signed int counter =-1; //start with negative one so that it begins with zero
bool increment = false;
//(global variable)

/*
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}




//TODO: implement the rest of the numbers (5-f)
// 7SEG layout:   abcdefg

int nums[16] = {0b1111110, // 0
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
                }; 

void outNum(int num){
	PORTD = nums[num] << 1; //assigns bits 1-7 of nums(a-f) to pins 2-7 of port d
  serial_println(PORTD);
  PORTB = SetBit(PORTB, 0 ,nums[num]&0x01); // assigns bit 0 of nums(g) to pin 0 of port b
}
*/

enum states {INIT, state_0, state_1, state_2 } state; //TODO: finish the enum for the SM


void Tick() {

  // State Transistions ///////////////////////////////////////
   
  switch(state) {

    case INIT:
    state = state_0;
    
      break;
    
    case state_0:
    if(((PINC >> 2)& 0x01)){
      state = state_1;
    }
    else if(!((PINC >>2) & 0x01)){
      state = state_0;
      
    }
    break;

    case state_1:
    if(((PINC >>2) & 0x01) && ((PINC >>3) & 0x01)){ //if both arent press (remember they are at 1, button pressed =0)
      state = state_1;
    }
    else if(((PINC >>2) & 0x01) && !((PINC >>3) & 0x01)){ //if A3 is pressed (go to reset state)
      state = state_2;
    }
    else if(!((PINC >>2) & 0x01) && ((PINC >>3) & 0x01)){ //if A2 is pressed go back to state_0 (increment)
      state = state_0;
    }
    
    break;

    case state_2:
    if(!((PINC >> 3)& 0x01)){ //is A3 is pressed stay in reset state
      state = state_2;
    }
    else if(((PINC >> 3)& 0x01)){ //if A3 is not pressed go back
      state = state_1;
    }
    break;

    default:
      state = INIT;
      break;

  }

  // State Actions //////////////////////////////////////////
  switch(state) {

    case INIT:        // nothing
      break;

    case state_0:     // check and increment 
    if(!increment && counter <= 15){
      counter++;
      increment = true;

    }
    else if(counter > 15){
      counter = 0;
    }
    
    
    switch (counter)
    {
    case 0:
      PORTB &= 0X00;
      //outNum(0);
      break;
    
    case 1:
      PORTB |= 0x04;
      //outNum(1);
      break;
    case 2: 
      PORTB &= 0X00;
      PORTB |= 0x08;
      //outNum(2);
      break;
    case 3: 
      PORTB &= 0X00; 
      PORTB |= 0x0C;
     // outNum(3);
      break;
    case 4:
      PORTB &= 0X00; 
      PORTB |= 0x10;
     // outNum(4);
      break;
      
    case 5: 
      PORTB &= 0X00;
      PORTB |= 0x14;
     // outNum(5);
      break;
    case 6: 
      PORTB &= 0X00;
      PORTB |= 0x18;
     // outNum(6);
      break;
    case 7: 
      PORTB &= 0X00; 
      PORTB |= 0x1C;
     // outNum(7);
      break;
    case 8: 
      PORTB &= 0X00; 
      PORTB |= 0x20;
     // outNum(8);
      break;
    case 9: 
      PORTB &= 0X00; 
      PORTB |= 0x24;
     // outNum(9);
      break;
    case 10: 
      PORTB &= 0X00; 
      PORTB |= 0x28;
     // outNum(10);
      break;
    case 11: 
      PORTB &= 0X00; 
      PORTB |= 0x2C;
     // outNum(11);
      break;
    case 12: 
      PORTB &= 0X00; 
      PORTB |= 0x30;
     // outNum(12);
      break;
    case 13: 
      PORTB &= 0X00; 
      PORTB |= 0x34;
      //outNum(13);
      break;
    case 14: 
      PORTB &= 0X00; 
      PORTB |= 0x38;
     // outNum(14);
      break;
    case 15: 
     PORTB &= 0X00; 
     PORTB |= 0x3C;
     //outNum(15);
      break;

    default:
    PORTB &= 0X00;
      break;
    }
    
     
    break;

    case state_1: //do nothing 
    
    increment = false;
   
    break;

    case state_2: //reset
    PORTB &= 0x00;
    counter =0;
    //outNum(0);
    
    break;

    default:
    PORTB &= 0x00;
    counter =0;
    //outNum(0);

    break;

        
    }



  }





int main(void)
{
  
	 //NOTE: For the buttons, they will be in the on position, however the button will cut that connection 

   DDRB = 0xFF; PORTB = 0x00; //Turns all of the D ports as outputs (to control an output,  make one of the ports/bits into 1)
   //DDRD = 0xFF; PORTD = 0x00; // for pt3
   DDRC = 0x00;  PORTC = 0xFF; //Turns all of the B  ports as inputs (to control an Input,  make one of the ports/bits into 0)
   state = INIT;


  //TODO: initialize your state

    while (1)
    {
      
		  Tick();      // Execute one SM tick
      
     }

    return 0;
}