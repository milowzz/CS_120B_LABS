
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"
#include "stdio.h" // Required for sprintf
#include "stdlib.h"



//TODO: declare variables for cross-task communication
bool button_pressed = false;
bool button_pressed_once = false;
unsigned char buzzer_counter = 1;
unsigned char old_pot_val = 15;
unsigned char pot_percentage;
unsigned char rev_hi_setting = false;
unsigned char rev_hi_setting_pt2 = false;

unsigned char rev_low_setting = false;
unsigned char rev_low_setting_pt2;

unsigned char neutral_setting = false;
unsigned char neutral_setting_pt2 = false;

unsigned char fwd_low_setting = false;
unsigned char fwd_low_setting_pt2 = false;

unsigned char fwd_hi_setting = false;
unsigned char fwd_hi_setting_pt2 = false;

const unsigned char temperature_threshold = 86;
unsigned char three_second_overheat_cnt = 0;


int tempReading;
double tempK;
unsigned char tempF;

char buffer[3];
char buffer_pt2[3];
unsigned char tempF_temporary;


long input_min = 0;
long input_max = 1023;
long output_min = 0;
long output_max =11;
unsigned char PWM_motor_counter =0;
double duty_cycle_percentage = 0.1;
/* TODO: match with how many tasks you have */
#define NUM_TASKS 6


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
const unsigned long buzzer_task_period = 100;
const unsigned long thermistor_task_period = 5;
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
int Tick_pot_percentage(int state);

enum PWM_MOTOR_States {PWM_MOTOR_INIT, PWM_MOTOR_PT1, PWM_MOTOR_PT2};
int Tick_pwm_motor(int state);

enum BUZZER_States {BUZZER_INIT, BUZZER_PT1, BUZZER_PT2, BUZZER_PT3, BUZZER_PT4};
int Tick_buzzer(int state);

enum THERMISTOR_States {THERMISTOR_INIT, THERMISTOR_PT1, THERMISTOR_PT2 };
int Tick_thermistor(int state);

 

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
    tasks[i].state = BUZZER_INIT;
    tasks[i].period = buzzer_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_buzzer;
    ++i;
    tasks[i].state = THERMISTOR_INIT;
    tasks[i].period = thermistor_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_thermistor;
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
        
         if(GetBit(PINC,1) == 1 && tempF < temperature_threshold){
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
            //button_pressed = true; 
            state = BUTTON_ON_RELEASE;
        }
    

        //  else if(button_pressed == true && tempF > temperature_threshold ){ //if system is on and temp is overthreshold
        //     if(three_second_overheat_cnt < 15){
        //         if((three_second_overheat_cnt / 2)%2 ==0){
        //         PORTB = SetBit(PORTB, 0, 1);
        //         three_second_overheat_cnt++;
        //         }
        //         else if((three_second_overheat_cnt / 2)%2 ==1){
        //         PORTB = SetBit(PORTB, 0, 0);
        //         three_second_overheat_cnt++; 
        //         }
        //     }
        //     else{
        //     PORTB = SetBit(PORTB, 0, 0);
        //     three_second_overheat_cnt =0;
        //     button_pressed = false;
        //     state = BUTTON_OFF;
        //     }
        // }
        
        
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

            lcd_goto_xy(1, 13);
            lcd_write_str(buffer);
            

            
        }
        else if(button_pressed == true){
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: On");

            lcd_goto_xy(1, 13);
            lcd_write_str(buffer);
            
            
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
            rev_hi_setting = true;

            rev_low_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
            // PORTB = SetBit(PORTB, 1, 0); 
            // PORTB = SetBit(PORTB, 2, 0);  

            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

               
              
            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
               
              
            }
        }
        else if(pot_percentage == 1){
             rev_hi_setting = true;

            rev_low_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 2){
             rev_hi_setting = true;

            rev_low_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 3){
            rev_low_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 4){
             rev_low_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Rev-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 5){
            neutral_setting = true;

            rev_hi_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
            rev_low_setting = false;
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 6){
             neutral_setting = true;

            rev_hi_setting = false;
            fwd_low_setting = false;
            fwd_hi_setting = false;
            rev_low_setting = false;
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Neutral");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 7){
            fwd_low_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_hi_setting = false;
            rev_low_setting = false;
             if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 8){
            fwd_low_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_hi_setting = false;
            rev_low_setting = false;
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Low");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
            }
        }
        else if(pot_percentage == 9){
            fwd_hi_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            rev_low_setting = false;
            if(button_pressed == true){ // this is to get of the extra % sign when going back down from 100%
            lcd_clear();
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: On");
            lcd_goto_xy(1,0);
            lcd_write_str("Fwd-Hi");
            lcd_goto_xy(1, 13);
            lcd_write_str(buffer);
            lcd_goto_xy(1, 15);
            lcd_write_character('%');
            }
            else{
            lcd_clear();
            lcd_goto_xy(0,0);
            lcd_write_str("Sys: Off");
            lcd_goto_xy(1,0);
            lcd_write_str("Fwd-Hi");
            lcd_goto_xy(1, 13);
            lcd_write_str(buffer);
            lcd_goto_xy(1, 15);
            lcd_write_character('%');

            }
        }
        else if(pot_percentage == 10){
            fwd_hi_setting = true;

            rev_hi_setting = false;
            neutral_setting = false;
            fwd_low_setting = false;
            rev_low_setting = false;
            if(button_pressed == true){
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: On");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');

            }
            else{
               lcd_clear();
               lcd_goto_xy(1,0);
               lcd_write_str("Fwd-Hi");
               lcd_goto_xy(0,0);
               lcd_write_str("Sys: Off");
               lcd_goto_xy(1, 13);
               lcd_write_str(buffer);
               lcd_goto_xy(1, 15);
               lcd_write_character('%');
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

int Tick_buzzer(int state){
//pot_percentage = map(ADC_read(0), input_min, input_max,  output_min, output_max);
    switch(state){
        case BUZZER_INIT:
        state = BUZZER_PT1;
        break;

        case BUZZER_PT1:

       

       if(GetBit(PINC, 1) == 1){
        if(buzzer_counter < 1){
            PORTB = SetBit(PORTB, 0, 1);
            buzzer_counter++;
        }
        else if(buzzer_counter >=1 ){
            PORTB = SetBit(PORTB, 0, 0);
            buzzer_counter=0;
            state = BUZZER_PT2;
        }
       }

       if(rev_hi_setting == true){
         rev_hi_setting_pt2 = true;  //indicates if we were in this setting before
       }

       if(rev_hi_setting == false){ //  checks if we moved to a new setting
        if(rev_hi_setting_pt2 == true){ // checks if we were in a old setting after moving to a new one
            if(buzzer_counter < 1){
                PORTB = SetBit(PORTB, 0, 1);
                buzzer_counter++;
        }
            else if(buzzer_counter >= 1 ){
                PORTB = SetBit(PORTB, 0, 0);
                buzzer_counter=0;
            state = BUZZER_PT3;
        }
       }
       }

       if(rev_low_setting == true){
        rev_low_setting_pt2 = true;
       }
       if(rev_low_setting == false){
        if(rev_low_setting_pt2 == true){
            if(buzzer_counter < 1){
                PORTB = SetBit(PORTB, 0, 1);
                buzzer_counter++;
        }
            else if(buzzer_counter >= 1 ){
                PORTB = SetBit(PORTB, 0, 0);
                buzzer_counter=0;
            state = BUZZER_PT3;
        }

        }
       }


       if(neutral_setting == true){
        neutral_setting_pt2 = true;
       }
       if(neutral_setting == false){
        if(neutral_setting_pt2 == true){
            if(buzzer_counter < 1){
                PORTB = SetBit(PORTB, 0, 1);
                buzzer_counter++;
        }
            else if(buzzer_counter >= 1 ){
                PORTB = SetBit(PORTB, 0, 0);
                buzzer_counter=0;
            state = BUZZER_PT3;
        }

        }
       }

       if(fwd_low_setting == true){
        fwd_low_setting_pt2 = true;
       }
       if(fwd_low_setting == false){
        if(fwd_low_setting_pt2 == true){
            if(buzzer_counter < 1){
                PORTB = SetBit(PORTB, 0, 1);
                buzzer_counter++;
        }
            else if(buzzer_counter >= 1 ){
                PORTB = SetBit(PORTB, 0, 0);
                buzzer_counter=0;
            state = BUZZER_PT3;
        }

        }
       }
       
       if(fwd_hi_setting == true){
        fwd_hi_setting_pt2 = true;
       }
       if(fwd_hi_setting == false){
        if(fwd_hi_setting_pt2 == true){
            if(buzzer_counter < 1){
                PORTB = SetBit(PORTB, 0, 1);
                buzzer_counter++;
        }
            else if(buzzer_counter >= 1 ){
                PORTB = SetBit(PORTB, 0, 0);
                buzzer_counter=0;
            state = BUZZER_PT3;
        }

        }
       }


        break;

        case BUZZER_PT2:
        if(GetBit(PINC, 1) == 1){
            state = BUZZER_PT2;
        }
        else if(GetBit(PINC, 1) == 0){
            state = BUZZER_PT1;
        }

        

        break;

        case BUZZER_PT3:
         if(rev_hi_setting_pt2 == true){
            rev_hi_setting_pt2 = false;
            state = BUZZER_PT1;
        }
        if(rev_low_setting_pt2 == true){
            rev_low_setting_pt2 = false;
            state = BUZZER_PT1;
        }
        if(neutral_setting_pt2 == true){
            neutral_setting_pt2 = false;
            state = BUZZER_PT1;
        }
        if(fwd_low_setting_pt2 == true){
            fwd_low_setting_pt2 = false;
            state = BUZZER_PT1;
        }
        if(fwd_hi_setting_pt2 == true){
            fwd_hi_setting_pt2 = false;
            state = BUZZER_PT1;
        }


        break;

        case BUZZER_PT4:

        break;


        default:
        state = BUTTON_INIT;
        break;
    }
    switch(state){

        case BUZZER_PT1:
        break;

        case BUZZER_PT2:
        break;

        case BUZZER_PT3:
        break;

        case BUZZER_PT4:
        break;

        default:
        break;

    }

    return state;
}

int Tick_thermistor(int state){
    switch(state){
        case THERMISTOR_INIT:
        state = THERMISTOR_PT1;
        break;

        case THERMISTOR_PT1:
        tempReading = ADC_read(2);
        tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
        tempK= 1/ (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK)) * tempK);
        tempF = ((tempK - 273.15)*1.8)+32;
        //serial_println(tempF);

                                    
    sprintf(buffer, "%.d", tempF); // Format and store the double in the buffer with 2 decimal places
    //serial_println(tempF);
    

        break;

        case THERMISTOR_PT2:
        break;

        default:
        state = THERMISTOR_INIT;
        break;

    }
    switch(state){
        case THERMISTOR_PT1:
        break;

        case THERMISTOR_PT2:
        break;

        default:
        break;

    }
    return state;
}
