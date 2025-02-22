
#include "timerISR.h"
#include "helper.h"
#include "periph.h"



//TODO: declare variables for cross-task communication
int distance;
double result;
bool button_pressed = false;

// TODO: Change this depending on which exercise you are doing.
// Exercise 1: 3 tasks
// Exercise 2: 5 tasks
// Exercise 3: 7 tasks
#define NUM_TASKS 3


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long sonar_task_period = 1000;
const unsigned long display_task_period = 1;
const unsigned long left_button_task_period = 200;
const unsigned long first_GCD = findGCD(sonar_task_period, display_task_period);
const unsigned long GCD_PERIOD = findGCD(first_GCD, left_button_task_period); // do this a second time to get the final GCS since there are three tasks

task tasks[NUM_TASKS]; // declared task array with NUM_TASKS amount of tasks

//TODO: Define, for each task:

// (1) enums and
enum SONAR_States {SONAR_INIT, SONAR_measure_CENTI, SONAR_measure_INCHES };
int Tick_sonar(int state);

enum DISPLAY_States {DISPLAY_INIT, DISPLAY_CENTI, DISPLAY_CENTI_DIGIT_TWO,  DISPLAY_INCHES};
int Tick_display(int state);

enum LEFT_BUTTON_States {LEFTBUTTON_INIT, LEFTBUTTON_CENTI_OFF , LEFTBUTTON_INCHES_ON, LEFTBUTTON_INCHES_ON_RELEASE, LEFTBUTTON_CENTI_OFF_RELEASE };
int Tick_leftbutton(int state);
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
    sonar_init(); // initializes sonar


    DDRB = 0XFE; PORTB = 0X00; 
    DDRD = 0xFF; PORTD = 0x00; 
    DDRC = 0XFC; PORTC = 0x03;

    PORTB = SetBit(PORTB, 0, 1); // echo pin 
    PORTC = SetBit(PORTC, 2, 1); // trigger pin 

    PORTB = SetBit(PORTB, 5, 1); // D1/ 13th pin  (off)
    PORTB = SetBit(PORTB, 4, 1); // D1/ 12th pin  (off)


    unsigned char i=0;

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].elapsedTime = ...
    // tasks[0].TickFct = &task1_tick_function;
    tasks[i].state = SONAR_INIT;
    tasks[i].period = sonar_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_sonar;
    ++i;
    tasks[i].state = DISPLAY_INIT;
    tasks[i].period = display_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_display;
    ++i;
    tasks[i].state = LEFTBUTTON_INIT;
    tasks[i].period = left_button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_leftbutton;
    

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}

int Tick_sonar(int state){
    switch(state){
        case SONAR_INIT:
        //add display here
        
        state = SONAR_measure_CENTI;
        break;

        case SONAR_measure_CENTI:
        distance = sonar_read();
        //add display here
        break;

        case SONAR_measure_INCHES:
        distance = sonar_read();
        distance = distance/2.54;
        //add display here
        break;

        default:
        state = SONAR_INIT;
        break;
    }

    switch(state){
        case SONAR_measure_CENTI:
        // add display here, maybe computation to output centimeters
        break;

        case SONAR_measure_INCHES:
        // add display here, maybe computation to output inches
        break;

        default:
        break;
    }
    return state;

}

int Tick_display(int state){
    switch(state){
        case DISPLAY_INIT: 
        state = DISPLAY_CENTI;
        break;

        case DISPLAY_CENTI:
        //serial_println(GetBit(PINC,0));
        
       if(button_pressed == true){
        //result = distance / 2.54;
        PORTB = SetBit(PORTB, 3, 1); // turn off digit 2
        outNum(static_cast<int>(distance/2.54) % 10);
        PORTB = SetBit(PORTB, 2, 0); // turn on digit 1

       }
       else{ //display CENTIMETERS if the button is not pressed
        PORTB = SetBit(PORTB, 3, 1); // turn off digit 2
        outNum(distance % 10);
        PORTB = SetBit(PORTB, 2, 0); // turn on digit 1

       }
        

        state = DISPLAY_CENTI_DIGIT_TWO;
        


        //add display here
        break;

        case DISPLAY_CENTI_DIGIT_TWO:
        if(button_pressed == true){
            PORTB = SetBit(PORTB, 2, 1); // turn off digit 1
            outNum((static_cast<int>(distance/2.54) / 10) % 10);
            PORTB = SetBit(PORTB, 3, 0); // turn on digit 2
        }
        else{
           PORTB = SetBit(PORTB, 2, 1); // turn off digit 1
           outNum((distance / 10) % 10);
           PORTB = SetBit(PORTB, 3, 0); // turn on digit 2
        }
        
        state = DISPLAY_CENTI;

        break;


        case DISPLAY_INCHES:
        //add display here
        break;

        default:
        state = DISPLAY_INIT;
        break;
    }

    switch(state){
        case DISPLAY_CENTI:
        // add display here, maybe computation to output centimeters
        break;

        case DISPLAY_INCHES:
        // add display here, maybe computation to output inches
        break;

        default:
        break;
    }
    return state;

}

int Tick_leftbutton(int state){
    switch(state){ //transitions 
        case LEFTBUTTON_INIT:
        state = LEFTBUTTON_CENTI_OFF;
        break;

        case LEFTBUTTON_CENTI_OFF:
        if(GetBit(PINC,0) == 1){
            button_pressed = true; // turn on
            //serial_println(button_pressed);
            state = LEFTBUTTON_INCHES_ON;
        }
        else{
            button_pressed = false; //turn off
            state = LEFTBUTTON_CENTI_OFF;
        }

        break;

        case LEFTBUTTON_INCHES_ON:
        if(GetBit(PINC, 0) !=1 ){
            button_pressed = true; //stay on
            state = LEFTBUTTON_INCHES_ON_RELEASE;
        }
        else{
            state = LEFTBUTTON_INCHES_ON; 
        }    
        break;

        case LEFTBUTTON_INCHES_ON_RELEASE:
        if(GetBit(PINC, 0) == 0 ){
            button_pressed = true; //stay on
            state = LEFTBUTTON_INCHES_ON_RELEASE;
        }
        else{
            button_pressed = false;
            state = LEFTBUTTON_CENTI_OFF_RELEASE;
            
        }
        
        break;

        case LEFTBUTTON_CENTI_OFF_RELEASE:
        if(GetBit(PINC, 0) == 0){
            button_pressed = false;
            state = LEFTBUTTON_CENTI_OFF;
        }
        else{
            button_pressed = true;
            state = LEFTBUTTON_CENTI_OFF_RELEASE;
        }
        break;

        default:
        state = LEFTBUTTON_INIT;
        break;
    }

    switch(state){ //actions 
        case LEFTBUTTON_CENTI_OFF:
        // add display here, maybe computation to output centimeters

        break;

        case LEFTBUTTON_INCHES_ON:
        // add display here, maybe computation to output inches
        break;

        case LEFTBUTTON_INCHES_ON_RELEASE:
        break;

        case LEFTBUTTON_CENTI_OFF_RELEASE:
        break;

        default:
        break;
    }
    return state;

}