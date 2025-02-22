
#include "timerISR.h"
#include "helper.h"
#include "periph.h"



//TODO: declare variables for cross-task communication
int distance;
double result;
double threshold_close = 8.0;
double threshold_far = 12.0;
bool button_pressed = false;
bool right_button_pressed = false;
bool right_button_pressed_pt2 = false;
bool blue_led_on_or_off = false;
unsigned char PWM_RED_TICK1_counter =0;
unsigned char PWM_GREEN_TICK1_counter =0;
short blue_led_counter = 0;

// TODO: Change this depending on which exercise you are doing.
// Exercise 1: 3 tasks
// Exercise 2: 5 tasks
// Exercise 3: 7 tasks
#define NUM_TASKS 7


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
const unsigned long PWM_RED_LED_period = 1;
const unsigned long PWM_GREEN_LED_period = 1;
const unsigned long right_button_task_period = 200;
const unsigned long BLINK_BLUE_LED_task_period = 250;

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

enum PWM_LED_States {PWM_RED_INIT, PWM_RED_TICK1, PWM_RED_TICK2 };
int Tick_PWM_RED(int state);

enum PWM_GREEN_LED_States {PWM_GREEN_INIT, PWM_GREEN_TICK1, PWM_GREEN_TICK2 };
int Tick_PWM_GREEN(int state);

enum RIGHT_BUTTON_States {RIGHTBUTTON_INIT, RIGHTBUTTON_OFF, RIGHTBUTTON_ON, RIGHTBUTTON_ON_RELEASE, RIGHTBUTTON_OFF_RELEASE };
int Tick_rightbutton(int state);

enum Blue_led_States {BLUELED_INIT, BLINK1, BLINK2 };
int Tick_blue_led(int state);
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
    ++i;
    tasks[i].state = PWM_RED_INIT;
    tasks[i].period = PWM_RED_LED_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_PWM_RED;
    ++i;
    tasks[i].state = PWM_GREEN_INIT;
    tasks[i].period = PWM_GREEN_LED_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_PWM_GREEN;
    ++i;
    tasks[i].state = RIGHTBUTTON_INIT;
    tasks[i].period = right_button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_rightbutton;
    ++i;
    tasks[i].state = BLUELED_INIT;
    tasks[i].period = BLINK_BLUE_LED_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_blue_led;
    

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

int Tick_PWM_RED(int state){
    switch(state){
        case PWM_RED_INIT:
        state = PWM_RED_TICK1;
        break;

        case PWM_RED_TICK1:

        if(distance < threshold_close || (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_close/2.54) % 10){ //duty cycle should be 100% if its less than 8cm or 3 inches 
           if(right_button_pressed_pt2 != true){//normal operation 
             PORTC = SetBit(PORTC, 3, 1); // RGB red on 
           } 
           else{ //if rightbutton is pressed 
             PORTC = SetBit(PORTC, 3, 0); // RGB red off
            
           }

        }
        
        else if((distance >= threshold_close && distance <= threshold_far) || (((static_cast<int>(distance/2.54) % 10) >= static_cast<int>(threshold_close/2.54) % 10  ) && (static_cast<int>(distance/2.54) % 10) <= static_cast<int>(threshold_far/2.54) % 10 )){
            if(right_button_pressed_pt2 != true){ // normal operation
            if(PWM_RED_TICK1_counter < 9){
               PORTC = SetBit(PORTC, 3, 1); // RGB red on 
               PWM_RED_TICK1_counter++;
               //serial_println(PWM_RED_TICK1_counter);
               
            }
            else{
                PWM_RED_TICK1_counter =0;
                state = PWM_RED_TICK2;
            }
            }
            else{// if rightbutton is pressed 
             PORTC = SetBit(PORTC, 3, 0); // RGB red off 
            }
            
        }

        else if(distance > threshold_far || ((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_far/2.54) % 10 )){
                if(right_button_pressed_pt2 != true){
                    PORTC = SetBit(PORTC, 3, 0); // RGB red off
                }
                else{
                    PORTC = SetBit(PORTC, 3, 0); // RGB red off
                }
                
        }

        break;

        case PWM_RED_TICK2:

        if(distance < threshold_close || (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_close/2.54) % 10){ //duty cycle should be 100% if its less than 8cm or 3 inches 
           if(right_button_pressed_pt2 != true){
            PORTC = SetBit(PORTC, 3, 1); // RGB red on
           }
           else{
            PORTC = SetBit(PORTC, 3, 0); // RGB red off
           }
           
        }

        else if((distance >= threshold_close && distance <= threshold_far) || (((static_cast<int>(distance/2.54) % 10) >= static_cast<int>(threshold_close/2.54) % 10  ) && (static_cast<int>(distance/2.54) % 10) <= static_cast<int>(threshold_far/2.54) % 10 )){
            if(right_button_pressed_pt2 != true){
                if(PWM_RED_TICK1_counter < 1){
               PWM_RED_TICK1_counter++;
               PORTC = SetBit(PORTC, 3, 0); // RGB red off
            }
            else{
                PWM_RED_TICK1_counter =0;
                state = PWM_RED_TICK1;
            }
            }
            else{
                PORTC = SetBit(PORTC, 3, 0); // RGB red off 
            }
            
        }

        else if(distance > threshold_far || ((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_far/2.54) % 10 )){
                if(right_button_pressed_pt2 != true){
                    PORTC = SetBit(PORTC, 3, 0); // RGB red off
                }
                else{
                    PORTC = SetBit(PORTC, 3, 0); // RGB red off
                }
                
        }

        break;

        default:
        state = PWM_RED_TICK1;
        break;


    }

    switch(state){
        case PWM_RED_INIT:
        break;

        case PWM_RED_TICK1:
        break;

        case PWM_RED_TICK2:
        break;

        default:
        break;


    }
     return state;
}

int Tick_PWM_GREEN(int state){
     switch(state){
        case PWM_GREEN_INIT:
        state = PWM_GREEN_TICK1;
        break;

        case PWM_GREEN_TICK1:

        if(distance < threshold_close || (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_close/2.54) % 10){ //duty cycle should be 100% if its less than 8cm or 3 inches 
           if(right_button_pressed_pt2 != true){ //operate as intented 
            PORTC = SetBit(PORTC, 4, 0); // RGB green off
           }
           else{ //operate if rightbutton is pressed
            PORTC = SetBit(PORTC, 4, 0); // RGB green off
           }
           
        }
        
        else if((distance > threshold_close && distance < threshold_far) || (((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_close/2.54) % 10  ) && (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_far/2.54) % 10 )){
            if(right_button_pressed_pt2 != true){// operate as intented 
            if(PWM_GREEN_TICK1_counter < 3){
               PORTC = SetBit(PORTC, 4, 1); // RGB green on 
               PWM_GREEN_TICK1_counter++;
            }
            else{
                PWM_RED_TICK1_counter =0;
                state = PWM_GREEN_TICK2;
            }
            }
            else{// operate if rightbutton is pressed
                PORTC = SetBit(PORTC, 4, 0);
            }
        }

        else if(distance > threshold_far || ((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_far/2.54) % 10 )){
                if(right_button_pressed_pt2 != true){ // operate as intented
                    PORTC = SetBit(PORTC, 4, 1); // RGB green on
                }
                else{ //operate in this condition if rightbutton is pressed 
                    PORTC = SetBit(PORTC, 4, 0); // RGB green on
                }
                
        }


       
        break;
        

        case PWM_GREEN_TICK2:
        if(distance < threshold_close || (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_close/2.54) % 10){ //duty cycle should be 100% if its less than 8cm or 3 inches 
           if(right_button_pressed_pt2 != true){ //operate as intented
            PORTC = SetBit(PORTC, 4, 0); // RGB green off
           } 
           else{ // operate if rightbutton is pressed 
            PORTC = SetBit(PORTC, 4, 0); // RGB green off
           }
        }

        else if((distance > threshold_close && distance < threshold_far) || (((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_close/2.54) % 10  ) && (static_cast<int>(distance/2.54) % 10) < static_cast<int>(threshold_far/2.54) % 10 )){
            if(right_button_pressed_pt2 != true){ //operate as intented 
            if(PWM_GREEN_TICK1_counter < 7){
               PORTC = SetBit(PORTC, 4, 0); // RGB green on
               PWM_GREEN_TICK1_counter++;
            }
            else{
                PWM_GREEN_TICK1_counter =0;
                state = PWM_GREEN_TICK1;
            }
            }
            else{
                PORTC = SetBit(PORTC, 4, 0);
            }
            
        }

        else if(distance > threshold_far || ((static_cast<int>(distance/2.54) % 10) > static_cast<int>(threshold_far/2.54) % 10 )){
                if(right_button_pressed_pt2 != true){
                    PORTC = SetBit(PORTC, 4, 1); // RGB green on
                } 
                else{
                    PORTC = SetBit(PORTC, 4, 0); // RGB green on
                }
        }
        
        
        break;

        default:
        state = PWM_GREEN_TICK1;
        break;


    }

    switch(state){
        case PWM_GREEN_INIT:
        break;

        case PWM_GREEN_TICK1:
        break;

        case PWM_GREEN_TICK2:
        break;

        default:
        break;


    }
     return state;
    
}

int Tick_rightbutton(int state){
     switch(state){ //transitions 
        case RIGHTBUTTON_INIT:
        state = RIGHTBUTTON_OFF;
        break;

        case RIGHTBUTTON_OFF:
        if(GetBit(PINC,1) == 1){
            right_button_pressed = true; // turn on
            //right_button_pressed_pt2 = true;
            
            threshold_close = distance * 0.8;
            threshold_far = distance * 1.2;
            state = RIGHTBUTTON_ON;
        }
        

        break;

        case RIGHTBUTTON_ON:
        if(GetBit(PINC, 1) == 0 ){
            right_button_pressed = true; //stay on
            //right_button_pressed_pt2 = false;
            state = RIGHTBUTTON_ON_RELEASE;
        }
        else{
            state = RIGHTBUTTON_ON; 
        }    
        break;

        case RIGHTBUTTON_ON_RELEASE:
        if(GetBit(PINC, 1) == 0 ){
            right_button_pressed = true; //stay on
            state = RIGHTBUTTON_ON_RELEASE;
        }
        else{
           right_button_pressed = false;
            state = RIGHTBUTTON_OFF_RELEASE;
            
        }
        
        break;

        case RIGHTBUTTON_OFF_RELEASE:
        if(GetBit(PINC, 1) == 0){
            right_button_pressed = false;
            state = RIGHTBUTTON_OFF;
        }
        else{
            right_button_pressed = true;
            state = RIGHTBUTTON_OFF_RELEASE;
        }
        break;

        default:
        state = RIGHTBUTTON_INIT;
        break;
    }

    switch(state){ //actions 
        case RIGHTBUTTON_OFF:
        // add display here, maybe computation to output centimeters
        break;

        case RIGHTBUTTON_ON:
        // add display here, maybe computation to output inches
        break;

        case RIGHTBUTTON_ON_RELEASE:
        break;

        case RIGHTBUTTON_OFF_RELEASE:
        break;

        default:
        break;
    }
    return state;

    
}

int Tick_blue_led(int state){
    switch(state){
        case BLUELED_INIT:
        state = BLINK1;
        break;


        case BLINK1:
        //serial_println(GetBit(PINC, 1) );
        if(GetBit(PINC, 1) == 1){
            right_button_pressed_pt2 = true;
        }
        if(right_button_pressed_pt2 == true){
            if(blue_led_counter < 12){
                if(blue_led_on_or_off == false){
                    PORTC = SetBit(PORTC, 5, 1);
                    blue_led_counter++;
                    //serial_println(blue_led_counter);
                    blue_led_on_or_off = true;
                }
                else if(blue_led_on_or_off == true){
                    PORTC = SetBit(PORTC, 5, 0);
                    blue_led_counter++;
                    blue_led_on_or_off = false;
                }
                
            }
            else{
                // reset variables
                right_button_pressed_pt2 =false;
                blue_led_counter=0;
                PORTC = SetBit(PORTC, 5, 0);
                blue_led_on_or_off = false;
                state = BLINK2;

            }
        }
            
        
        break;

        case BLINK2:
    
        state = BLINK1;
        break;

        default:
        state = BLUELED_INIT;
        break;
    }

    switch(state){
        case BLINK1:
        break;

        case BLINK2:
        break;

        default:
        break;

    }
    return state;


}