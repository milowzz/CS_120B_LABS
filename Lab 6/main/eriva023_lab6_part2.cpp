
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"



//TODO: declare variables for cross-task communication
bool button_pressed = false;
unsigned char pot_percentage;
long input_min = 0;
long input_max = 1023;
long output_min = 0;
long output_max =11;
unsigned char PWM_motor_counter =0;
double duty_cycle_percentage = 0.1;
/* TODO: match with how many tasks you have */
#define NUM_TASKS 4


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long button_task_period = 200;
const unsigned long lcd_display_task_period= 1000;
const unsigned long POT_task_period = 500;
const unsigned long motor_task_period = 5;
unsigned long TEMP_dutycycle_POT_task_period = POT_task_period;
//const unsigned long first_GCD = findGCD(button_task_period, lcd_display_task_period);
const unsigned long GCD_PERIOD = 5;

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
enum BUTTON_States {BUTTON_INIT, BUTTON_OFF , BUTTON_ON, BUTTON_ON_RELEASE, BUTTON_OFF_RELEASE };
int Tick_button(int state);

enum LCD_display_States {DISPLAY_INIT, DISPLAY_OFF, DISPLAY_ON};
int Tick_display(int state);

enum POT_PERCENTAGE_States {POT_PERCENTAGE_INIT, POT_PERCENTAGE_PT1, POT_PERCENTAGE_PT2}; //pretty sure I can do this in one state for PT1
int Tick_pot_percentage(int states);

enum PWM_MOTOR_States {PWM_MOTOR_INIT, PWM_MOTOR_PT1, PWM_MOTOR_PT2};
int Tick_pwm_motor(int states);

 

// (2) tick functions

void TimerISR() {
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

    DDRB = 0XFF; PORTB = 0X00; //outputs
    DDRD = 0xFF; PORTD = 0x00; //outputs 
    DDRC = 0X00; PORTC = 0xFF; //inputs 

    //serial_init(9600);
    lcd_init();
    
    

    //TODO: Initialize tasks here
    // e.g. tasks[0].period = TASK1_PERIOD
    // tasks[0].state = ...
    // tasks[0].elapsedTime = ...
    // tasks[0].TickFct = &task1_tick_function;
    
    unsigned char i=0;

    tasks[i].state = BUTTON_INIT;
    tasks[i].period = button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_button;
    ++i;
    tasks[i].state = DISPLAY_INIT;
    tasks[i].period = lcd_display_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_display;
    ++i;
    tasks[i].state = POT_PERCENTAGE_INIT;
    tasks[i].period = POT_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_pot_percentage;
    ++i;
    tasks[i].state = PWM_MOTOR_INIT;
    tasks[i].period = motor_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_pwm_motor;
    ++i;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}

//Feel free to implement your tasks' tick functions under here
int Tick_button(int state){
    switch(state){
        case BUTTON_INIT:
        state = BUTTON_OFF;
        break;

        case BUTTON_OFF:
        
         if(GetBit(PINC,1) == 1){
            button_pressed = true; // turn on
            state = BUTTON_ON;
        }
        else{
            button_pressed = false;
            state = BUTTON_OFF;
        }
        break;

        case BUTTON_ON:
         if(GetBit(PINC,1) == 0){
            button_pressed = true; 
           
           
            state = BUTTON_ON_RELEASE;
        }
        else{
            state = BUTTON_ON;
        }
        break;

        case BUTTON_ON_RELEASE:
         if(GetBit(PINC,1) == 0){
            // PORTB = SetBit(PORTB, 1, 1); WORKS
            // PORTB = SetBit(PORTB, 2, 0); WORKS 
            button_pressed = true; 
            state = BUTTON_ON_RELEASE;
        }
        else{
            button_pressed = false;
            state = BUTTON_OFF_RELEASE;
        }
        break;

        case BUTTON_OFF_RELEASE:
         if(GetBit(PINC,1) == 0){
            button_pressed = false; 
            state = BUTTON_OFF;
        }
        else{
            state = BUTTON_OFF_RELEASE;
        }
        break;

        default:
        state = BUTTON_INIT;
        break;

    }

    switch(state){
        case BUTTON_INIT:
        break;

        case BUTTON_OFF:
        break;

        case BUTTON_ON:
        break;

        case BUTTON_ON_RELEASE:
        break;

        case BUTTON_OFF_RELEASE:
        break;

        default:
        break;


    }
    return state;

}

int Tick_display(int state){
    switch(state){
        case DISPLAY_INIT:
        state = DISPLAY_OFF;
        break;

        case DISPLAY_OFF: //display off is just showing the display for right now
        if(button_pressed == false){
            lcd_clear();
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: Off");
            PORTB = SetBit(PORTB, 1, 0); 
            PORTB = SetBit(PORTB, 2, 0);
        }
        else if(button_pressed == true){
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: On");
            
        }
        break;

        case DISPLAY_ON:
        break;

        default:
        state = DISPLAY_INIT;
        break;

    }
    switch(state){

        case DISPLAY_INIT:

        break;

        case DISPLAY_OFF:
        break;

        case DISPLAY_ON:
        break;

        default:
        break;
        

    }
    return state;
}
int Tick_pot_percentage(int state){
pot_percentage = map(ADC_read(0), input_min, input_max,  output_min, output_max);
    switch(state){
        case POT_PERCENTAGE_INIT:
        state = POT_PERCENTAGE_PT1;
        break;

        case POT_PERCENTAGE_PT1:
        if(pot_percentage == 0){
            // PORTB = SetBit(PORTB, 1, 0); 
            // PORTB = SetBit(PORTB, 2, 0);  

            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 1){
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 2){
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 3){
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 4){
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 5){
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 6){
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 7){
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 8){
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }
        else if(pot_percentage == 9){
            if(button_pressed == true){ // this is to get of the extra % sign when going back down from 100%
            lcd_clear();
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: On");
            lcd_goto_xy(1,0);
            lcd_write_str("Fwd-Hi");
            }
            else{
            lcd_clear();
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: Off");
            lcd_goto_xy(1,0);
            lcd_write_str("Fwd-Hi");

            }
        }
        else if(pot_percentage == 10){
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
            }
        }

        break;

        case POT_PERCENTAGE_PT2:
        break;

        default:
        state = POT_PERCENTAGE_INIT;
        break;

    }
    switch(state){
        case POT_PERCENTAGE_INIT:
        break;

        case POT_PERCENTAGE_PT1:
       
        break;
        
        case POT_PERCENTAGE_PT2:

      
        break;

        default:
        break;


    }
    return state;
}

int Tick_pwm_motor(int state){
    switch(state){
        case PWM_MOTOR_INIT:
        state = PWM_MOTOR_PT1;
        break;

        case PWM_MOTOR_PT1: // MOTOR IS ON IN THIS STATE
        //  if(pot_percentage == 0 && button_pressed == true){
        //     PORTB = SetBit(PORTB, 1, 1);  // max duty cycle in reverse 
        //     PORTB = SetBit(PORTB, 2, 0);
          
        // }
        // else if(pot_percentage == 1 && button_pressed == true){
        //         PORTB = SetBit(PORTB, 1, 1); // max duty cycle in reverse 
        //         PORTB = SetBit(PORTB, 2, 0);
        // }
        if(pot_percentage <= 2 && button_pressed == true){
                PORTB = SetBit(PORTB, 1, 1); // max duty cycle in reverse 
                PORTB = SetBit(PORTB, 2, 0);
        }
        else if(pot_percentage == 3 && button_pressed == true){
            if(PWM_motor_counter < 4){
                PORTB = SetBit(PORTB, 1, 1); // low duty cycle in reverse
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT2;
            }
        }
        else if(pot_percentage == 4 && button_pressed == true){
            if(PWM_motor_counter < 4){
                PORTB = SetBit(PORTB, 1, 1); // low duty cycle in reverse
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT2;
            }
        }
        else if(pot_percentage == 5 && button_pressed == true){
                PORTB = SetBit(PORTB, 1, 0); //motor is off (netural state)
                PORTB = SetBit(PORTB, 2, 0);  
        }
        else if(pot_percentage == 6 && button_pressed == true){
                PORTB = SetBit(PORTB, 1, 0); //motor is off (netural state)
                PORTB = SetBit(PORTB, 2, 0);  
            
        }
        else if(pot_percentage == 7 && button_pressed == true){
            if(PWM_motor_counter < 4){
                PORTB = SetBit(PORTB, 1, 0); //motor is on forward (low duty cycle)
                PORTB = SetBit(PORTB, 2, 1);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT2;
            }
        }
        else if(pot_percentage == 8 && button_pressed == true){
            if(PWM_motor_counter < 4){
                PORTB = SetBit(PORTB, 1, 0); //motor is on forward (low duty cycle)
                PORTB = SetBit(PORTB, 2, 1);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT2;
            }
        }
        else if(pot_percentage == 9 && button_pressed == true){
            PORTB = SetBit(PORTB, 1, 0); // Fwd-hi
            PORTB = SetBit(PORTB, 2, 1);  
        }
        if(pot_percentage == 10 && button_pressed == true){ // 100% duty cycle 
            PORTB = SetBit(PORTB, 1, 0); //Fwd-hi
            PORTB = SetBit(PORTB, 2, 1);  
        }
       
        break;//////////////////////////////////////////////////////////////////////////////////////

        case PWM_MOTOR_PT2: // MOTOR IS OFF IN THIS STATE
        //    if(pot_percentage == 1 && button_pressed == true){
        //     if(PWM_motor_counter< 9){
        //         PORTB = SetBit(PORTB, 1, 0); //motor is off
        //         PORTB = SetBit(PORTB, 2, 0);  
        //         PWM_motor_counter++;
        //     }
        //     else{
        //         PWM_motor_counter =0;
        //         state = PWM_MOTOR_PT1;
        //     }

        // }
        //  else if(pot_percentage == 2 && button_pressed == true){
        //     if(PWM_motor_counter < 8){
        //         PORTB = SetBit(PORTB, 1, 0); //motor is off
        //         PORTB = SetBit(PORTB, 2, 0);  
        //         PWM_motor_counter++;
        //     }
        //     else{
        //         PWM_motor_counter =0;
        //         state = PWM_MOTOR_PT1;
        //     }

        // }

         if(pot_percentage == 3 && button_pressed == true){
            if(PWM_motor_counter < 1){
                PORTB = SetBit(PORTB, 1, 0); //motor is off
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT1;
            }

        }
        else if(pot_percentage == 4 && button_pressed == true){
            if(PWM_motor_counter < 1){
                PORTB = SetBit(PORTB, 1, 0); //motor is off
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT1;
            }

        }
        else if(pot_percentage == 5 && button_pressed == true){
                PORTB = SetBit(PORTB, 1, 0); //motor is off (neutral)
                PORTB = SetBit(PORTB, 2, 0);  
        }
        else if(pot_percentage == 6 && button_pressed == true){
                PORTB = SetBit(PORTB, 1, 0); //motor is off (neutral)
                PORTB = SetBit(PORTB, 2, 0);  

        }
        else if(pot_percentage == 7 && button_pressed == true){
            if(PWM_motor_counter < 1){
                PORTB = SetBit(PORTB, 1, 0); //motor is off
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT1;
            }

        }
        else if(pot_percentage == 8 && button_pressed == true){
            if(PWM_motor_counter < 1){
                PORTB = SetBit(PORTB, 1, 0); //motor is off
                PORTB = SetBit(PORTB, 2, 0);  
                PWM_motor_counter++;
            }
            else{
                PWM_motor_counter =0;
                state = PWM_MOTOR_PT1;
            }

        }
        // else if(pot_percentage == 9 && button_pressed == true){
        //     if(PWM_motor_counter < 1){
        //         PORTB = SetBit(PORTB, 1, 0); //motor is off
        //         PORTB = SetBit(PORTB, 2, 0);  
        //         PWM_motor_counter++;
        //     }
        //     else{
        //         PWM_motor_counter =0;
        //         state = PWM_MOTOR_PT1;
        //     }

        // }
        // if(pot_percentage == 10 && button_pressed == true){ // 100% duty cycle 
        //     PORTB = SetBit(PORTB, 1, 1); 
        //     PORTB = SetBit(PORTB, 2, 0);  
        // }
      
        break;

        default:
        state = PWM_MOTOR_INIT;
        break;
    }

    switch(state){

        case PWM_MOTOR_INIT:
        break;

        case PWM_MOTOR_PT1:
        break;

        case PWM_MOTOR_PT2:
        break;

        default:
        break;

    }
    return state;

}