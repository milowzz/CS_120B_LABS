#include "timerISR.h"
#include "helper.h"
#include "periph.h"




//declare global variables
unsigned char light_cnt =0;
unsigned char amber_light_cnt =0;
unsigned long police_light_cnt =0;
unsigned long police_siren_cnt =0;
bool reverse_gear = false;
bool amber_lights = false;
bool police_lights = false;
bool police_siren = false;
bool joystick_pressed = false;
unsigned int adc_val_y;
unsigned int adc_val_x;
unsigned long input_min = 16;
unsigned long input_max = 1024;
unsigned long output_min = 0;
unsigned long output_max = 13;
unsigned char joystick_movement_y_axis;
unsigned char joystick_movement_x_axis;
unsigned char i =0;
unsigned long j =0;



 //map(ADC_read(1), input_min, input_max, output_min_photoR, output_max_photoR)
#define NUM_TASKS 8 //TODO: Change to the number of tasks being used


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
const unsigned long left_button_task_period = 200;
const unsigned long right_button_task_period = 200;
const unsigned long joystick_button_task_period = 200;
const unsigned long amber_lights_task_period = 1;
const unsigned long police_lights_task_period = 1;
const unsigned long joystick_task_period = 1;
const unsigned long passive_buzzer_task_period = 100;
const unsigned long stepper_motor_task_period = 1;



// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 1;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Declare your tasks' function and their states here
enum LEFT_BUTTON_States {LEFTBUTTON_INIT, LEFTBUTTON_OFF , LEFTBUTTON_ON_AMBER_LIGHT, LEFTBUTTON_ON_RELEASE_AMBER_LIGHT, LEFTBUTTON_OFF_RELEASE_AMBER_LIGHT, LEFTBUTTON_ON_POLICE_LIGHT, LEFTBUTTON_ON_RELEASE_POLICE_LIGHT, LEFTBUTTON_OFF_RELEASE_POLICE_LIGHT };
int Tick_leftbutton(int state);

enum Amber_lights_States {AMBER_LIGHTS_INIT, WAIT, AMBER_LIGHTS_ON, AMBER_LIGHTS_OFF};
int Tick_amber_lights(int state);

enum Police_lights_States {POLICE_LIGHTS_INIT, WAIT_PL, POLICE_LIGHTS_ON, POLICE_LIGHTS_OFF};
int Tick_police_lights(int state);

enum Joystick_States {JOYSTICK_INIT, JOYSTICK_PT1};
int Tick_joystick(int state);

enum Joystick_button_States {JOYSTICK_BUTTON_INIT, JOYSTICK_BUTTON_OFF, JOYSTICK_BUTTON_ON, JOYSTICK_BUTTON_ON_RELEASE, JOYSTICK_BUTTON_OFF_RELEASE};
int Tick_joystick_button(int state);

enum passive_buzzer_States {PASSIVE_BUZZER_INIT, PASSIVE_BUZZER_PT1, PASSIVE_BUZZER_PT2};
int Tick_passive_buzzer(int state);

enum stepper_motor_States {STEPPER_MOTOR_INIT, STEPPER_MOTOR_PT1, STEPPER_MOTOR_PT2};
int Tick_stepper_motor(int state);

enum right_button_States {RIGHTBUTTON_INIT, RIGHTBUTTON_OFF, RIGHTBUTTON_ON, RIGHTBUTTON_ON_RELEASE,RIGHTBUTTON_OFF_RELEASE };
int Tick_right_button(int state);

void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; // Stepper motor phases

//TODO: Create your tick functions for each task



int main(void) {
    //TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
    DDRB = 0xFF; PORTB = 0x00; // PORTB are outputs 
    DDRD = 0xFF; PORTD = 0x00; // PORTD are outputs 
    DDRC = 0x00; PORTC = 0xFF; // PORTC are inputs 


    //TODO: Initialize the buzzer timer/pwm(timer0)

    //TODO: Initialize the servo timer/pwm(timer1)

     unsigned char i=0;
    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;

    tasks[i].state = LEFTBUTTON_INIT;
    tasks[i].period = left_button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_leftbutton;
    ++i;
    tasks[i].state = AMBER_LIGHTS_INIT;
    tasks[i].period = amber_lights_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_amber_lights;
    ++i;
    tasks[i].state = POLICE_LIGHTS_INIT;
    tasks[i].period = police_lights_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_police_lights;
    ++i;
    tasks[i].state = JOYSTICK_INIT;
    tasks[i].period = joystick_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_joystick;
    ++i;
    tasks[i].state = JOYSTICK_BUTTON_INIT;
    tasks[i].period = joystick_button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_joystick_button;
    ++i;
    tasks[i].state = PASSIVE_BUZZER_INIT;
    tasks[i].period = passive_buzzer_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_passive_buzzer;
    ++i;
    tasks[i].state = STEPPER_MOTOR_INIT;
    tasks[i].period = stepper_motor_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_stepper_motor;
    ++i;
    tasks[i].state = RIGHTBUTTON_INIT;
    tasks[i].period = right_button_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_right_button;
    ++i;


    TimerSet(GCD_PERIOD);
    TimerOn();


    while (1) {}
    

    return 0;
}

int Tick_leftbutton(int state){
  switch(state){
    case LEFTBUTTON_INIT:
    state = LEFTBUTTON_OFF;
    break;

    case LEFTBUTTON_OFF:
    if (GetBit(PINC, 3) == 0) { // Button is not pressed
      if (light_cnt > 0 && light_cnt < 5) { // Button was held down for a short duration
        light_cnt = 0; // Reset variable
        amber_lights = true; // Activate amber lights on release
        state = LEFTBUTTON_ON_RELEASE_AMBER_LIGHT;
      }
      else if (light_cnt >= 5) { // Button was held down for a longer duration
        light_cnt = 0; // Reset variable
        police_lights = true;
        state = LEFTBUTTON_ON_RELEASE_POLICE_LIGHT;
      } 
      else {
        state = LEFTBUTTON_OFF; // Stay in off state if no valid press
      }
    } 
    else if (GetBit(PINC, 3) == 1) { // Button is pressed
    light_cnt++; // Increment counter while the button is pressed
    }
    break;

    case LEFTBUTTON_ON_RELEASE_AMBER_LIGHT:
    if(GetBit(PINC,3) == 0){ // stay
      state = LEFTBUTTON_ON_RELEASE_AMBER_LIGHT;
    }
    else if(GetBit(PINC,3) == 1){
      state = LEFTBUTTON_OFF_RELEASE_AMBER_LIGHT;
    }
    break;

    case LEFTBUTTON_OFF_RELEASE_AMBER_LIGHT:
    if(GetBit(PINC,3) == 1){// stay
        state = LEFTBUTTON_OFF_RELEASE_AMBER_LIGHT;
    }
    else if(GetBit(PINC,3) == 0){
      amber_lights = false; //reset variable
      state = LEFTBUTTON_OFF;
    }
    break;

    case LEFTBUTTON_ON_RELEASE_POLICE_LIGHT:
     if(GetBit(PINC,3) == 0){ // stay
      state = LEFTBUTTON_ON_RELEASE_POLICE_LIGHT;
    }
    else if(GetBit(PINC,3) == 1){
      state = LEFTBUTTON_OFF_RELEASE_POLICE_LIGHT;
    }
    break;

    case LEFTBUTTON_OFF_RELEASE_POLICE_LIGHT:
     if(GetBit(PINC,3) == 1){// stay
        state = LEFTBUTTON_OFF_RELEASE_POLICE_LIGHT;
    }
    else if(GetBit(PINC,3) == 0){
      police_lights = false; //reset variable
      state = LEFTBUTTON_OFF;
    }

    break;

    

    default:
    state = LEFTBUTTON_INIT;
    break;

  }

  switch(state){

    case LEFTBUTTON_OFF:
    break;

    case LEFTBUTTON_ON_RELEASE_AMBER_LIGHT:
    break;

    case LEFTBUTTON_OFF_RELEASE_AMBER_LIGHT:
    break;

    case LEFTBUTTON_ON_RELEASE_POLICE_LIGHT:
    break;

    case LEFTBUTTON_OFF_RELEASE_POLICE_LIGHT:
    break;

    default:
    break;

  }
  return state;

}



int Tick_amber_lights(int state){
  switch(state){
    case AMBER_LIGHTS_INIT:
    state = WAIT;
    break;

    case WAIT:
    if(amber_lights == true){  // stay here and check if amber_lights is true from leftbutton_tick
      state = AMBER_LIGHTS_ON;
    }
    PORTD = SetBit(PORTD, 4, 0); // turn it off
    PORTD = SetBit(PORTD, 3, 0); // turn it off
    break;

    case AMBER_LIGHTS_ON:
    //serial_println(amber_light_DC);
    if(amber_lights == false){
      state = WAIT;
    }

  
    // NOTE: we dont need to do PWM as the lab doesnt not require it
    else if (amber_lights == true) {
      if (amber_light_cnt < 300) { // Total counts to complete two blinks and a pause

        // Blink twice (each blink cycle is 50 counts)
        if ((amber_light_cnt < 50) || (amber_light_cnt >= 100 && amber_light_cnt < 150)) {
            PORTD = SetBit(PORTD, 4, 1); // Turn it on
            PORTD = SetBit(PORTD, 3, 1); // Turn it on
        } else {
            PORTD = SetBit(PORTD, 4, 0); // Turn it off
            PORTD = SetBit(PORTD, 3, 0); // Turn it off
        }

        amber_light_cnt++; 
    } 
    else {
        amber_light_cnt = 0;  // Reset variable after loop completes
        state = AMBER_LIGHTS_OFF; // Move on to the next state as needed
    }
    }

    break;

    case AMBER_LIGHTS_OFF:
    //serial_println(amber_light_cnt);
    if(amber_lights == false){ // if button is pressed turn off the lights 
      state = WAIT;
    }
    else{
        amber_light_cnt =0;
        state = AMBER_LIGHTS_ON;
      }
    
    break;

    default:
    state = AMBER_LIGHTS_INIT;
    break;


  }
  switch(state){ // action

    case WAIT:
    break;

    case AMBER_LIGHTS_ON:
    break;

    case AMBER_LIGHTS_OFF:
    break;

    default:
    break;

  }
  return state;

}

int Tick_police_lights(int state){
  switch(state){
    case POLICE_LIGHTS_INIT:
    state = WAIT_PL;
    break;

    case WAIT_PL:
    if(police_lights == true){  // stay here and check if amber_lights is true from leftbutton_tick
      state = POLICE_LIGHTS_ON;
    }
    
    break;

    case POLICE_LIGHTS_ON:
    if(police_lights == false){
      state = WAIT_PL;
    }

    else if (police_lights == true) {
    // Each color blinks three times, alternating between blue and red
    if (police_light_cnt < 600) { // Total counts needed for alternating blinks (3 for each color)
    // Alternate between blue and red every 3 blinks (each blink lasting 100 counts)
    if ((police_light_cnt / 100) % 6 < 3) { // First 3 blinks are blue
        if (police_light_cnt % 100 < 30) {
            PORTD = SetBit(PORTD, 2, 1); // Turn blue on
            PORTD = SetBit(PORTD, 4, 0); // Ensure red is off
        } else {
            PORTD = SetBit(PORTD, 2, 0); // Turn blue off
        }
    } else { // Next 3 blinks are red
        if (police_light_cnt % 100 < 30) {
            PORTD = SetBit(PORTD, 4, 1); // Turn red on
            PORTD = SetBit(PORTD, 2, 0); // Ensure blue is off
        } else {
            PORTD = SetBit(PORTD, 4, 0); // Turn red off
        }
    }
    police_light_cnt++; // Increment the counter
} else {
    police_light_cnt = 0; // Reset after three full blinks for each color
    state = POLICE_LIGHTS_OFF; // Move to the next state as needed
}

}
    break;


    case POLICE_LIGHTS_OFF:
    if(police_lights == false){ // if button is pressed turn off the lights 
      state = WAIT_PL;
    }
    else{
      police_light_cnt =0;
      state = POLICE_LIGHTS_ON;
    }
    break;


    default:
    state = POLICE_LIGHTS_INIT;
    break;

  }
  switch(state){
    case WAIT_PL:
    break;

    case POLICE_LIGHTS_ON:
    break;

    case POLICE_LIGHTS_OFF:
    break;

    default:
    break;

  }
  return state;
}

int Tick_joystick(int state){
  switch(state){
    case JOYSTICK_INIT:
    state = JOYSTICK_PT1;
    break;

    case JOYSTICK_PT1:
    //adc_val_y = ADC_read(1);  // Read joystick y-axis
    joystick_movement_y_axis = map(ADC_read(1), input_min, input_max, output_min, output_max);
    joystick_movement_x_axis = map(ADC_read(0), input_min, input_max, output_min, output_max);
    

    break;

    default:
    state = JOYSTICK_INIT;
    break;


  }

  switch(state){

    case JOYSTICK_PT1:
    break;

    default:
    break;

  }

  return state;
}
int Tick_joystick_button(int state){
 
  switch(state){
    case JOYSTICK_BUTTON_INIT:
    state = JOYSTICK_BUTTON_OFF;
    break;

    case JOYSTICK_BUTTON_OFF:
    if(GetBit(PINC, 2) != 0){ // if not pressed
      joystick_pressed = false;
       //serial_println(police_siren);
      state = JOYSTICK_BUTTON_OFF;
    }
    else{
      joystick_pressed = true;
      state = JOYSTICK_BUTTON_ON;
    }
    break;

    case JOYSTICK_BUTTON_ON:
    if(GetBit(PINC, 2) == 0){ // if it is pressed
      joystick_pressed = true;
      state = JOYSTICK_BUTTON_ON;
    }
    else{
      state = JOYSTICK_BUTTON_ON_RELEASE;
    }
    break;

    case JOYSTICK_BUTTON_ON_RELEASE:
    if(GetBit(PINC, 2) == 0){
      joystick_pressed = false;
      state = JOYSTICK_BUTTON_OFF_RELEASE;
    }
    else{
      joystick_pressed = true;
      state = JOYSTICK_BUTTON_ON_RELEASE;
    }
    break;

    case JOYSTICK_BUTTON_OFF_RELEASE:
    if(GetBit(PINC, 2) == 0){
      state = JOYSTICK_BUTTON_OFF_RELEASE;
    }
    else{
      state = JOYSTICK_BUTTON_OFF;
    }
    break;

    

    default:
    state = JOYSTICK_BUTTON_INIT;
    break;

  }
  switch(state){

    case JOYSTICK_BUTTON_OFF:
    break;

    case JOYSTICK_BUTTON_ON:
    break;

    case JOYSTICK_BUTTON_ON_RELEASE:
    break;

    case JOYSTICK_BUTTON_OFF_RELEASE:
    break;

    default:
    break;
    
  }

  return state;
}

int Tick_passive_buzzer(int state){
  //serial_println(police_siren_cnt);
  switch(state){
    case PASSIVE_BUZZER_INIT:
    state = PASSIVE_BUZZER_PT1;
    break;

    case PASSIVE_BUZZER_PT1:
    if(joystick_pressed == true && police_lights == true){//if true make the buzzer beep
    OCR0A = 128; //sets duty cycle to 50% since TOP is always 256

    TCCR0A |= (1 << COM0A1);// use Channel A
    TCCR0A |= (1 << WGM01) | (1 << WGM00);// set fast PWM Mode
      if(police_siren_cnt < 2){
        TCCR0B = (TCCR0B & 0xF8) | 0x04;//set prescaler to 256 low note 
        police_siren_cnt++;
      }
      else{
        police_siren_cnt=0;
        state = PASSIVE_BUZZER_PT2;
      }
    
    // TCCR0B = (TCCR0B & 0xF8) | 0x03;//set prescaler to 64
    // TCCR0B = (TCCR0B & 0xF8) | 0x04;//set prescaler to 256
    // TCCR0B = (TCCR0B & 0xF8) | 0x05;//set prescaler to 1024
    }
    else{
      OCR0A = 255;

    }
    break;

    case PASSIVE_BUZZER_PT2:
    if(joystick_pressed == true && police_lights == true){
      if(police_siren_cnt < 2){
        TCCR0B = (TCCR0B & 0xF8) | 0x03;//set prescaler to 64 high note
        police_siren_cnt++;
      }
      else{
        police_siren_cnt=0;
        state = PASSIVE_BUZZER_PT1;
      }
      
    }
    else{
      OCR0A = 255;
    }

    break;

    default:
    state = PASSIVE_BUZZER_INIT;
    break;

  }
  switch(state){

    case PASSIVE_BUZZER_PT1:
    break;

    case PASSIVE_BUZZER_PT2:
    break;

    default:
    break;
    
  }
  return state;
}

int Tick_stepper_motor(int state){
  switch(state){
    case STEPPER_MOTOR_INIT:
    state = STEPPER_MOTOR_PT1;
    break;

    case STEPPER_MOTOR_PT1:
    if (reverse_gear == false) {
    // Check if the joystick is at position 5, and don't move the motor if true
    if (joystick_movement_x_axis == 5) {
        // Skip updating the motor if joystick_movement_x_axis is 5
        break; // Exit the block early without moving the motor
    }

    // Check if j has reached the threshold
    if (j >= (10 - joystick_movement_x_axis) ) {
        // Move the stepper motor to the next phase only if j has reached the threshold
        PORTB = (PORTB & 0x03) | (stages[i] << 2);
        i++;
        if (i > 7) {
            i = 0;
        }
        j = 0; // Reset j after moving the motor
    } else {
        j++; // Increment j until it reaches the threshold
    }
}

    else if(reverse_gear != false){
      if (joystick_movement_x_axis == 5) {
        // Skip updating the motor if joystick_movement_x_axis is 5
        break; // Exit the block early without moving the motor
    }

    // Check if j has reached the threshold
    if (j >= (10 - joystick_movement_x_axis) ) {
        // Move the stepper motor to the next phase only if j has reached the threshold
        PORTB = (PORTB & 0x03) | stages[i] << 2;
      i--;
      if(i<0 || i>8){
        i = 7;
      }
        j = 0; // Reset j after moving the motor
    } else {
        j++; // Increment j until it reaches the threshold
    }
      
    }
    
    break;

    case STEPPER_MOTOR_PT2:
    break;

    default:
    state = STEPPER_MOTOR_INIT;
    break;

  }

  switch(state){
    case STEPPER_MOTOR_PT1:
    break;

    case STEPPER_MOTOR_PT2:
    break;

    default:
    break;
    
  }
  return state;
}

int Tick_right_button(int state){
  switch(state){
    case RIGHTBUTTON_INIT:
    state = RIGHTBUTTON_OFF;
    break;

    case RIGHTBUTTON_OFF:
    if(GetBit(PINC, 4) == 0){//stay 
      PORTD = SetBit(PORTD, 5, 0);
      reverse_gear = false;
      state = RIGHTBUTTON_OFF;
    }
    else if(GetBit(PINC, 4) == 1){ // otherwise if button is pressed move to next state
      reverse_gear = true;
      PORTD = SetBit(PORTD, 5, 1);
      state = RIGHTBUTTON_ON;
    }
    
    break;

    case RIGHTBUTTON_ON:
    if(GetBit(PINC, 4) == 1){//stay
      state = RIGHTBUTTON_ON;
    }
    else if(GetBit(PINC, 4) == 0){
      state = RIGHTBUTTON_ON_RELEASE;
    }

    break;

    case RIGHTBUTTON_ON_RELEASE:
    if(GetBit(PINC, 4) == 0){// stay
    state = RIGHTBUTTON_ON_RELEASE;
    }
    else if(GetBit(PINC, 4) == 1){
      PORTD = SetBit(PORTD, 5, 0);
      state = RIGHTBUTTON_OFF_RELEASE;
    }
    break;

    case RIGHTBUTTON_OFF_RELEASE:
    if(GetBit(PINC, 4) == 1){// stay
    state = RIGHTBUTTON_OFF_RELEASE;
    }
    else if(GetBit(PINC, 4) == 0){
      reverse_gear = false;
      state = RIGHTBUTTON_OFF;
    }
    break;

    default:
    state = RIGHTBUTTON_INIT;
    break;

  }
  switch(state){

    case RIGHTBUTTON_OFF:
    //PORTD = SetBit(PORTD, 5, 0);
    break;

    case RIGHTBUTTON_ON:
    //PORTD = SetBit(PORTD, 5, 1);
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