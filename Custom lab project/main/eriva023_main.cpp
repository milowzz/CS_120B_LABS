#include "eriva023_timerISR.h"
#include "eriva023_helper.h"
#include "eriva023_periph.h"
#include "eriva023_irAVR.h"
#include "eriva023_serialATmega.h"
#include "eriva023_dealer.h"
#include "time.h"
#include "eriva023_spi.h"
#include "eriva023_ST7735_LCD.h"

#define DECK_SIZE 20
#define HAND_SIZE 5


//declare global variables
decode_results results;
uint16_t seed = 0;
unsigned char timer =0;
uint16_t tones[] = {1000, 800, 600, 400, 300};
uint8_t num_tones = sizeof(tones) / sizeof(tones[0]);
volatile uint8_t tone_step = 0;      // Current step in the tone sequence
volatile uint16_t current_icr = 0;  // Current ICR1 value
bool play_buzzer = false;
unsigned char A3_counter = 0;
unsigned char roulette_test;

unsigned char buzzer_counter =0;
bool power_button = false;
bool reset_button = false;
bool call_liar_button = false;
bool turngame_backon = false;
int age = 25;

 //map(ADC_read(1), input_min, input_max, output_min_photoR, output_max_photoR)
#define NUM_TASKS 3 //TODO: Change to the number of tasks being used


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
const unsigned long IR_sensor_task_period = 100;
const unsigned long passive_buzzer_task_period = 200;
const unsigned long dealer_task_period = 200;
const unsigned long display_one_task_period = 200;
const unsigned long startup_screen_task_period = 200;

// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 100;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Declare your tasks' function and their states here

enum startup_screen_States {STARTUP_SCREEN_INIT, STARTUP_SCREEN_PT1, STARTUP_SCREEN_PT2, POWER_OFF};
int Tick_startup_screen(int state);

enum IR_sensor_States {IR_sensor_INIT, IR_SENSOR_READ, IR_SENSOR_READ_PT2};
int Tick_ir_sensor(int state); 

enum passive_buzzer_States {PASSIVE_BUZZER_INIT, PASSIVE_BUZZER_WAIT, PASSIVE_BUZZER_PLAY_SAD_TUNE};
int Tick_passive_buzzer(int state);





void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}




//TODO: Create your tick functions for each task
int main(void) {
    //TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
   
    for (int i = 0; i < 16; i++) {
      seed = (seed << 1) | (ADC_read(0) & 0x01); // Use ADC LSB as noise
    }
    srand(seed);

    IRinit(&DDRC, &PINC, 1);
    serial_init(9600);
    DDRB = 0xFF; PORTB = 0x00; // PORTB are outputs 
    DDRD = 0xFF; PORTD = 0x00; // PORTD are outputs 
    DDRC = 0x00; PORTC = 0xFF; // PORTC are inputs 
    SPI_INIT();
    ST7735_init();


    unsigned char i=0;
    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;

    tasks[i].state = STARTUP_SCREEN_INIT;
    tasks[i].period = startup_screen_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_startup_screen;
    ++i;
    tasks[i].state = IR_sensor_INIT;
    tasks[i].period = IR_sensor_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_ir_sensor;
    ++i;
    tasks[i].state = PASSIVE_BUZZER_INIT;
    tasks[i].period = passive_buzzer_task_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &Tick_passive_buzzer;
    ++i;
    // tasks[i].state = DISPLAY_ONE_INIT;
    // tasks[i].period = display_one_task_period;
    // tasks[i].elapsedTime = tasks[i].period;
    // tasks[i].TickFct = &Tick_display_one;
    // ++i;
    // tasks[i].state = DEALER_INIT;
    // tasks[i].period = display_one_task_period;
    // tasks[i].elapsedTime = tasks[i].period;
    // tasks[i].TickFct = &Tick_display_one;
    // ++i;
   
    

    TimerSet(GCD_PERIOD);
    TimerOn();


    while (1) {}
    

    return 0;
}


int Tick_startup_screen(int state){
    switch(state){
        case STARTUP_SCREEN_INIT:
        SetCSToLow_display_two(); //trun on communicationm for display two
        SetCSToHigh_display_one(); //turn off communicatoon for display one
        Send_Command(0x2A);  // CASET
        Send_data(0x00);     // Start column (high byte, 0x00 for 0)
        Send_data(0x00);     // Start column (low byte, 0x00 for 0)
        Send_data(0x00);     // End column (high byte, 0x00 for 128)
        Send_data(0x81);     // End column (low byte, 0x80 for 128)

        // Set the drawing area (Row Address Set)
        Send_Command(0x2B);  // RASET
        Send_data(0x00);     // Start row (high byte, 0x00 for 0)
        Send_data(0x00);     // Start row (low byte, 0x00 for 0)
        Send_data(0x00);     // End row (high byte, 0x00 for 128)
        Send_data(0x81);     // End row (low byte, 0x81 for 129)

        // Begin memory write
        Send_Command(0x2C);  // RAMWR

        // Fill the entire screen with red color
        for (long j = 0; j < (129 * 128); j++) {  // Rectangle size: 129x128 pixels
            Send_data(0x00);  // Make the whole display black
            Send_data(0x00);  
            Send_data(0x00); 
        }

        shuffle(deck, deckSize);
        dealCards(deck, &deckSize, player1, &p1HandSize);
        
        //display_screen_two();
        new_table();
        table_name(table_name_);
        printHand("PLAYERONE", player1, p1HandSize);
       
       
       
        SetCSToHigh_display_two(); // end communication for display two

        SetCSToLow_display_one();  // turn on communication for display one
        SetCSToHigh_display_two(); // turn off communication for display two

        Send_Command(0x2C);  // RAMWR

        for (long j = 0; j < (129 * 128); j++) {  // Rectangle size: 129x128 pixels
        Send_data(0x00);  // Make the whole display black
        Send_data(0x00);  
        Send_data(0x00); 
        }

        shuffle(deck, deckSize);
        dealCards(deck, &deckSize, player2, &p2HandSize);
        table_name(table_name_);
        serial_println("table_name_");
        serial_println(table_name_);
        printHand("PLAYERTWO", player2, p2HandSize);

        

        SetCSToHigh_display_one(); // end communication for display one

       //NOTE CONTINUE THE GAME OVER HERE AFTER INTIALIZING!
       SetCSToLow_display_two(); //turn on communicationm for display two
       SetCSToHigh_display_one(); //turn off communicatoon for display one

       // GAME START HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       //Player_one's turn
       
        state = STARTUP_SCREEN_PT1;

        break;

        case STARTUP_SCREEN_PT1:
        playerone_turn();
        Player1_turn(player1, p1HandSize);
        
       
    if (GetBit(PINC, 3) == 1) { // Button is pressed
        A3_counter++; // Increment the counter for each tick
    }    
       else if (GetBit(PINC, 3) == 0 && A3_counter > 0) { // Button is released
        if (A3_counter >= 5) {
            // Long press action
            A3_counter = 0;
             DrawString(122, 10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
             SetCSToHigh_display_two(); // turn off communication for display two
             SetCSToLow_display_one();  // turn on communication for display one

            state = STARTUP_SCREEN_PT2;
        }   
        else {
            // Short press action
            cards_selected(temp_array, player1, p1HandSize); 
            clear_screen();
            printHand("PLAYERONE", player1, p1HandSize);
            pixel_start = 110;
            }

       // Reset the counter after handling the press
       A3_counter = 0;
    } 

    if(power_button == true){
      state = POWER_OFF;
    }

      
    
        break;

        case STARTUP_SCREEN_PT2: //player two's turn
        //serial_println("hi");
        // SetCSToHigh_display_two(); // turn off communication for display two
        // SetCSToLow_display_one();  // turn on communication for display one
  
        playertwo_turn();
        Player2_turn(player2, p2HandSize); 

        if (GetBit(PINC, 3) == 1) { // Button is pressed
        A3_counter++; // Increment the counter for each tick
        }    
       else if (GetBit(PINC, 3) == 0 && A3_counter > 0) { // Button is released
        if (A3_counter >= 5) {
            // Long press action
            A3_counter = 0;
             DrawString(122, 10, "\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\",0xFFFF, 0x0000);
             SetCSToLow_display_two(); //turn on communicationm for display two
             SetCSToHigh_display_one(); //turn off communicatoon for display one
            state = STARTUP_SCREEN_PT1;
        }   
        else {
            // Short press action
            cards_selected(temp_array_pt2, player2, p2HandSize); 
            clear_screen();
            printHand("PLAYERTWO", player2, p2HandSize);
            pixel_start = 110;
       }

       // Reset the counter after handling the press
       A3_counter = 0;
    } 
        

        break;


        case POWER_OFF:
        turn_gameOff();
        if(power_button == false){
           state = STARTUP_SCREEN_INIT;
        }
        break;


        default:
        state = STARTUP_SCREEN_INIT;
        break;
    }

    switch(state){
        case STARTUP_SCREEN_PT1:
        break;

        case STARTUP_SCREEN_PT2:
        break;

        default:
        break;

    }

    return state;
}

int Tick_ir_sensor(int state){
    switch(state){
        case IR_sensor_INIT:
        PORTD = SetBit (PORTD, 6, 1);
        state = IR_SENSOR_READ;
        break;

        case IR_SENSOR_READ:

        IRdecode(&results);
        
       if(results.value == 16753245 && power_button == false){
        power_button = true;
        turngame_backon = false;
        PORTD = SetBit (PORTD, 6, 0); //turns the game off
       }
       if(results.value == -1 && power_button == true){ // make a bool that checks if the power button was pressed prior to this aka 16753245
        power_button = false;
        turngame_backon = true;
        PORTD = SetBit (PORTD, 6, 1); //turns the game on again
       }
    //    if(results.value == 16769565 && reset_button == false){
    //     reset_button = true;
    //     PORTD = SetBit (PORTD, 4, 1); // resets the game 
    //    }
    //    if(results.value == -1 && reset_button == true){
    //     reset_button = false;
    //     PORTD = SetBit (PORTD, 4, 0); //turns the game on again
    //    }
       if(results.value == 16738455 && call_liar_button == false){ // button 0 
        roulette_number_pt2 = rand() % 4 + 1;
        //roulette_number = rand() % 4
        if(player1_turn == true){
            //call_liar(temp_array, table_name_, p1HandSize);
             call_liar(temp_array_pt2, table_name_, p2HandSize);
        }
        else if(player2_turn == true){
            //call_liar(temp_array_pt2, table_name_, p2HandSize);
             call_liar(temp_array, table_name_, p1HandSize);
        }
        call_liar_button = true;
        
        
       }
        if(results.value == -1 && call_liar_button == true){
        // serial_println("roulette");
        // serial_println(roulette_number_pt2);
        call_liar_button = false;
       
        
       }
       serial_println(results.value);
    //    serial_println("p1");
    //    serial_println(player1_turn);
    //    serial_println("p2");
    //    serial_println(player2_turn);

       //just make another if statement for another button press from the remote to reset the game to the beginning at anytime
       state = IR_SENSOR_READ_PT2;
       
     
        break;

        case IR_SENSOR_READ_PT2:
        IRresume();
        //PORTD = SetBit (PORTD, 6, 0);
        state = IR_SENSOR_READ;
        break;

        default:
        state = IR_sensor_INIT;
        break;

    }

    switch(state){

        case IR_SENSOR_READ:
        break;

        case IR_SENSOR_READ_PT2:
        break;

        default:
        break;
        
    }
    return state;
}

int Tick_passive_buzzer(int state){
    switch(state){
        case PASSIVE_BUZZER_INIT:
        state = PASSIVE_BUZZER_WAIT;
        break;

        case PASSIVE_BUZZER_WAIT: //NOTE: This works!
      if(play_sad_buzzer == true && buzzer_counter < 1){ // if the player loses aka, the routtlet hits, play the sound and display the winner/loser on screen
            state = PASSIVE_BUZZER_PLAY_SAD_TUNE;
       }
       if(buzzer_counter > 1){
        play_sad_buzzer = false;
        buzzer_counter =0;
       }

        break;

        case PASSIVE_BUZZER_PLAY_SAD_TUNE:
        buzzer_counter++;
        TCCR1A |= (1 << WGM11) | (1 << COM1A1);      // Fast PWM, non-inverting mode
        TCCR1B |= (1 << WGM12) | (1 << WGM13);       // Fast PWM with ICR1 as TOP
        TCCR1B |= (1 << CS11);                       // Prescaler 8

        ICR1 = 39999;                                // Initial TOP value (50 Hz for servo control)

        // Configure PB1 (OC1A) as output
        DDRB |= (1 << PB1);

    
        if (tone_step < num_tones) {
            current_icr = F_CPU / (8 * tones[tone_step]) - 1;
            ICR1 = current_icr;
            OCR1A = current_icr / 2; // 50% duty cycle
            tone_step++;
            }   
        else {
        // Stop PWM after the sequence
        TCCR1A = 0;
        TCCR1B = 0;
        //TIMSK2 = 0; // Disable Timer2 interrupt DONT DO THIS AS IT WILL DISABLE ALL TIMERS AND MESS WITH THE PROGRAM
        tone_step=0;
        play_buzzer = false; 
        state = PASSIVE_BUZZER_WAIT; 
        }
        
       
        break;

        default:
        state = PASSIVE_BUZZER_INIT;
        break;

    }

    switch(state){

        case PASSIVE_BUZZER_WAIT:
        
        break;

        case PASSIVE_BUZZER_PLAY_SAD_TUNE:
        break;

        default:
        break;
        
        
    }

    return state;
}

// int Tick_dealer(int state){
//      switch(state){
//         case DEALER_INIT:
      
//         break;

//         case DEALER_PT1:
        

       
//         break;

//         case DEALER_PT2:
//         break;

//         default:
//         state = DEALER_INIT;
//         break;



//     }

//     switch(state){

//         case DEALER_PT1:
//         break;

//         case DEALER_PT2:
//         break;

//         default:
//         break;
        
//     }

//     return state;

// }



// int Tick_display_one(int state){
//     switch(state){
//         case DISPLAY_ONE_INIT:
       
//         break;

//         case DISPLAY_ONE_PT1:
      
//         break;

//         case DISPLAY_ONE_PT2:
    
//          state = DISPLAY_ONE_PT3;
     
//         break;

//         case DISPLAY_ONE_PT3:
    
//         break;


//         case DISPLAY_ONE_PT4:
    
//         break;

//         default:
//         state = DISPLAY_ONE_INIT;
//         break;

//     }

//     switch(state){

//         case DISPLAY_ONE_PT1:
        
//         break;

//         case DISPLAY_ONE_PT2:
//         break;

//         case DISPLAY_ONE_PT3:
//         break;

//         case DISPLAY_ONE_PT4:
//         break;
        

//         default:
//         break;
        
//     }

//     return state;
// }


