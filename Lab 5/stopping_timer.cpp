#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "serialATmega.h"


//TODO: declare variables for cross-task communication
bool start_timer_button = false;  // button to start the timer 


// TODO: Change this depending on which exercise you are doing.
// Exercise 1: 3 tasks
// Exercise 2: 5 tasks
// Exercise 3: 7 tasks
#define NUM_TASKS 2


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>

const unsigned long display_task_period = 1;  //7 segment display
const unsigned long timer_task_period = 1000;


const unsigned long GCD_PERIOD = 1; // might want to use the GCD function, if the lowest task period is not 1

task tasks[NUM_TASKS]; // declared task array with NUM_TASKS amount of tasks

//TODO: Define, for each task:

// (1) enums and


enum DISPLAY_States {DISPLAY_INIT, DISPLAY_WAIT, DISPLAY_START_TIMER, DISPLAY_END_TME, DISPLAY_RESET};
int Tick_display(int state);

enum TIMER_States {TIMER_INIT, TIMER_WAIT, START_TIMER, RESET_TIMER};
int Tick_timer(int state);

;
// (2) tick functions

void TimerISR() {
    
    //TODO: sample inputs here

	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int main(void) {
    //TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
    serial_init(9600);
    
    DDRB = 0XFE; PORTB = 0X00; //outputs 
    DDRD = 0xFF; PORTD = 0x00; //outputs 
    DDRC = 0XFC; PORTC = 0x03; //inputs 

  
   


    unsigned char i=0;

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].elapsedTime = ...
    // tasks[0].TickFct = &task1_tick_function;
  
    tasks[i].state = DISPLAY_INIT;
    tasks[i].period = display_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_display;
    ++i;
    tasks[i].state = TIMER_INIT;
    tasks[i].period = timer_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_timer;

   

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}



int Tick_display(int state){
    switch(state){
        case DISPLAY_INIT: 
        serial_println("HELLO");
        state = DISPLAY_WAIT;
        break;

        case DISPLAY_WAIT:

        PORTB = SetBit(PORTB, 3, 1); // turn off digit 3
        PORTB = SetBit(PORTB, 2, 1); // turn off digit 2
        outNum(1);
        PORTD = SetBit(PORTD, 6, 0); // turn on digit 1
        serial_println("hi_1");
        //state = DISPLAY_START_TIMER;
        break;

        case DISPLAY_START_TIMER:
        PORTB = SetBit(PORTB, 3, 1); // turn off digit 3
        PORTB = SetBit(PORTB, 1, 1); // turn off digit 1
        outNum(2);
        PORTB = SetBit(PORTB, 2, 0); // turn on digit 2
         serial_println("hi_2");
        state = DISPLAY_END_TME;
        break;


        case DISPLAY_END_TME:
         PORTB = SetBit(PORTB, 1, 1); // turn off digit 1
         PORTB = SetBit(PORTB, 2, 1); // turn off digit 2
         outNum(3);
         //PORTB = SetBit(PORTB, 3, 0); // turn on digit 3
         serial_println("hi_3");
        state = DISPLAY_WAIT;
        break;

        case DISPLAY_RESET:
        state = DISPLAY_INIT;
        break;

        default:
        state = DISPLAY_INIT;
        break;
    }

    switch(state){
        case DISPLAY_WAIT:
        
        break;

        case DISPLAY_START_TIMER:
        
        break;

        case DISPLAY_END_TME:
        
        break;

        case DISPLAY_RESET:
        
        break;

        default:
        break;
    }
    return state;

}


int Tick_timer(int state){
    switch(state){
        case TIMER_INIT: 
        state = TIMER_WAIT;
        break;

        case TIMER_WAIT:
        break;

        case START_TIMER:
        break;

        case RESET_TIMER:
        break;

        default:
        state = TIMER_INIT;
        break;
    }

    switch(state){
        case TIMER_WAIT:
        
        break;

        case START_TIMER:
        
        break;

        case RESET_TIMER:
        
        break;

      
        default:
        break;
    }
    return state;

}



