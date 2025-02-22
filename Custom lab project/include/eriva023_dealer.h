#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eriva023_serialATmega.h"
#include "eriva023_spi.h"
#include "eriva023_write_to_screen.h"


#define DECK_SIZE 20
#define HAND_SIZE 5

unsigned char pixel_start = 110;
unsigned char pixel_start_pt2= 100;
bool button_state = false;
bool button_state_pt2 = false;
unsigned char current_card = 0;
unsigned char current_card_pt2 = 0;
unsigned char size_pt1 =0;
bool select = false;
bool select_pt2 = false;
unsigned char counter_hand=0;
unsigned char counter_hand_pt2 =0;
unsigned char time_held =0;
bool player1_turn = false;
bool player2_turn = false;


int temp_array[3] = {0, 0, 0};
int temp_array_pt2[3] = {0, 0, 0};
int roulette[4] = {0, 0, 0, 0};
bool play_sad_buzzer = false;
unsigned char roulette_number;
unsigned char roulette_number_pt2;
unsigned char  valueToRemove = 0;
unsigned AMT_Selected= 0;
int indexToRemove =-1;
int currentValue;
//unsigned char temp_counter=0;

// Center pile
int center[DECK_SIZE] = {0};
int centerCount = 0;
// Hands for two players
int player1[HAND_SIZE] = {0}, player2[HAND_SIZE] = {0};
int p1HandSize = 0, p2HandSize = 0;
// Deck size
int deckSize = DECK_SIZE;
unsigned char c = 122;
unsigned char t = 110;
unsigned char button_pressed =0;
unsigned char button_pressed_pt2 =0;
unsigned char button_pressed_pt3 =0;
unsigned char table_name_;
unsigned long seed_pt2=0;


char buffer[50];

 // Initialize the deck with 6 Aces, 6 Kings, 6 Queens, and 2 Jokers
 int deck[DECK_SIZE] = {
        1, 1, 1, 1, 1, 1,  // Aces
        2, 2, 2, 2, 2, 2,  // Kings
        3, 3, 3, 3, 3, 3,  // Queens
        4, 4               // Jokers
    };

// Function to shuffle the deck
void shuffle(int deck[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        //serial_println(rand());
        int temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
    
}

//NOTE: need to implement a way to check the card placed down for a play
//i.e. if a player places down a king on an ace table and the other player thinks they are
//lying they should be able to press a button to confirm that

void new_table(){
   
    table_name_ =  rand() % 3 + 1;
    roulette_number = rand() % 4 + 1;
    roulette_number_pt2 = rand() % 4 + 1;
    //serial_println("beginning ");
    //serial_println(roulette_number);
    //serial_println(roulette_number_pt2);
    roulette[roulette_number] = 1;
}

void table_name(unsigned char table_name_){
     switch (table_name_) {
            case 1: DrawString(122, 30, "ACES[TABLE",0xFFFF, 0x0000); break;
            case 2: DrawString(122,30, "KINGS[TABLE",0xFFFF, 0x0000); break;
            case 3: DrawString(122,30, "QUEENS[TABLE",0xFFFF, 0x0000); break;
           
            default:  break; // Default case for unexpected values
        }
}

void play_roulette (){
    

    //roulette_number_pt2 = rand() % 4 + 1;
    serial_println("roulette");
    serial_println(roulette_number);
    serial_println(roulette_number_pt2);
    if( roulette[roulette_number_pt2] == 1 ){
        play_sad_buzzer = true;
        if(player1_turn && !player2_turn){ //player 1 loses and player 2 wins
            SetCSToHigh_display_one(); //turn off communicatoon for display one
            SetCSToHigh_display_two(); //turn off communicatoon for display one
            SetCSToLow_display_two(); //trun on communicationm for display two
            SetCSToHigh_display_one(); //turn off communicatoon for display one
            clear_whole_screen();
            DrawString(75,65, "YOU WIN",0xFFFF, 0x0000);
            clear_whole_screen();
            DrawString(75,65, "YOU WIN",0xFFFF, 0x0000);
            clear_whole_screen();
            SetCSToHigh_display_two(); //trun on communicationm for display two
            SetCSToLow_display_one(); //turn off communicatoon for display one
            clear_whole_screen();
            DrawString(75,65, "YOU LOSE",0xFFFF, 0x0000);
            clear_whole_screen();
            DrawString(75,65, "YOU LOSE",0xFFFF, 0x0000);
            clear_whole_screen();
            
        }
        if(!player1_turn && player2_turn){ //player 2 loses and player 1 wins
            SetCSToHigh_display_two(); 
            SetCSToLow_display_one(); 
            clear_whole_screen();
            DrawString(75,65, "YOU WIN",0xFFFF, 0x0000);
            SetCSToLow_display_two(); 
            SetCSToHigh_display_one(); 
            clear_whole_screen();
            DrawString(75,65, "YOU LOSE",0xFFFF, 0x0000);
            
        }
        
    }
  
}

void call_liar(int temp_array[], unsigned char table_name_, int size ){ //NOTE: need to pass in which player's turn in the argument
      for (int j = 0; j < 3; j++) {
        //serial_println(temp_array[j]);
        if(temp_array[j] != table_name_){
            if(temp_array[j] == 4){
            // do nothing
               // serial_println("JOKER");
            }
            else if(temp_array[j] == 0){
              // do nothing
              //serial_println("hihi");
            }
            else{
                //print loser or sum 
               // serial_println("ROULETTE");
                play_roulette();
            }


        }
        else if(temp_array[j] == table_name_){
            //serial_println("safe");
            //play_roulette();
        }
        
    }

}



void cards_selected(int temp_array[], int player1hand[], int& size) {
     // Track the current index in temp_array
    
    // Validate the valueToRemove index
    // if (valueToRemove >= 3) {
    //     serial_println("All values in temp_array processed");
    //     return;
    // }

    // Get the current value from temp_array
     currentValue = temp_array[valueToRemove];

    // Find and remove the value from player1hand
    

    for (int i = 0; i < size; i++) {
        if (player1hand[i] == currentValue) {
            indexToRemove = i;
            break;
        }
    }

    // If the value was found, remove it
    if (indexToRemove != -1) {
        for (int i = indexToRemove; i < size - 1; i++) {
            player1hand[i] = player1hand[i + 1];
        }
        size--; // Decrease the logical size of the array
    } else {
        //serial_println("Value not found in player1hand");
    }

    // Print the updated array
    serial_println("Updated player1hand:");
    for (int j = 0; j < size; j++) {
        serial_println(player1hand[j]);
    }

    // Move to the next value in temp_array
    valueToRemove++;
}



//Function to print a hand
void printHand(const char *player, int hand[], int size) {
    snprintf(buffer, sizeof(buffer), "%sS HAND", player);
    DrawString(122, 120, buffer, 0xFFFF, 0x0000);
   
    for (int y = 0; y < size; y++) {
        pixel_start-= 10;
        switch (hand[y]) {
            case 1: DrawString(122,pixel_start, "ACE",0xFFFF, 0x0000); break;
            case 2: DrawString(122,pixel_start, "KING",0xFFFF, 0x0000); break;
            case 3: DrawString(122,pixel_start, "QUEEN",0xFFFF, 0x0000); break;
            case 4: DrawString(122,pixel_start, "JOKER",0xFFFF, 0x0000); break;
            default: DrawString(122,pixel_start, "INVAILD",0xFFFF, 0x0000); break; // Default case for unexpected values
        }
    }

    if(pixel_start == 60){
        pixel_start = 110;
    }

}

void playerone_turn(){
     SetCSToLow_display_two(); //turn on communicationm for display two
     SetCSToHigh_display_one(); //turn off communicatoon for display one
    
     DrawString(122, 10, "PLAYER+ONES+TURN",0xFFFF, 0x0000);

    player1_turn =true;
    player2_turn =false;

}
void playertwo_turn(){
     SetCSToHigh_display_two(); //turn on communicationm for display two
     SetCSToLow_display_one(); //turn off communicatoon for display one
      DrawString(122, 10, "PLAYER+TWOS+TURN",0xFFFF, 0x0000);
    player1_turn =false;
    player2_turn =true;

}


 void Player1_turn(int hand[], int size) {
    // Wait for a button press
    //turns the game on again
    if (GetBit(PINC, 0) == 0 && time_held < 5 ) {
        if(GetBit(PINC, 2) == 1 && select == false ){
            switch (hand[current_card]) {
            case 1: 
            temp_array[counter_hand] = 1;
            AMT_Selected++;
            // serial_println("hi1");
            serial_println("ACE_1");
            break;
            case 2: 
            
            temp_array[counter_hand] =2;
            AMT_Selected++;
            // serial_println("hi2");
            serial_println("KING_1");

            break;
            case 3: 
            
            //serial_println("QUEEN");
            temp_array[counter_hand] =3;
            serial_println("QUEEN_1");
            //serial_println("hi3");
            AMT_Selected++;

             break;
            case 4: 
            //serial_println("JOKER");
            serial_println("JOKER_1");
             temp_array[counter_hand] = 4;
           
             break;
            
            }
            select = true;

            
        }
        if(GetBit(PINC, 2) == 0 && select == true){
            select =false;
            counter_hand++;
        }

        if(GetBit(PINC, 2) == 1 && time_held < 5){
            _delay_ms(200);
            time_held++;
            if(GetBit(PINC, 2) == 0){
                time_held=0;
            }
        }
        else if(time_held >= 5){
            time_held=0;
            counter_hand=0;
            temp_array[3] =0;

        }
        
      
    }

    // Button is pressed; proceed with the rest of the logic
    button_state = true;
   
    // Increment and check bounds
    
button_pressed_pt2++;
// Reset button cycle when it reaches 6
if (button_pressed_pt2 == 6) {
    button_pressed_pt2 = 1;
}


// Loop to move the "_" character while the button is pressed

if(GetBit(PINC, 0) == 1 ) {
    //Erase previous character
    
    // DrawString(90, pixel_start_pt2, "\\", 0xFFFF, 0x0000);

    // // Update position
    // pixel_start_pt2 -= 10;
    

    // // Draw new character
    // DrawString(90, pixel_start_pt2, " ", 0xFFFF, 0x0000);

    // // Loop back to starting position if out of bounds
    // if(button_pressed_pt2 == 1){
    //   DrawString(90, 60, "\\", 0xFFFF, 0x0000);
    // }
    // if (pixel_start_pt2 <= 60) {
    //     pixel_start_pt2 = 110;
    // }

    
    current_card++;
    if (current_card >= size) {
    current_card = 0; // Loop back to the beginning
    }

     switch (hand[current_card]) {
        case 1: DrawString(122, 50, "SELECT[ACE\\\\", 0xFFFF, 0x0000);  break;
        case 2: DrawString(122, 50, "SELECT[KING\\\\", 0xFFFF, 0x0000); break;
        case 3: DrawString(122, 50, "SELECT[QUEEN", 0xFFFF, 0x0000); break;
        case 4: DrawString(122, 50, "SELECT[JOKER", 0xFFFF, 0x0000); break;
        //default: DrawString(122, 50, "INVALID", 0xFFFF, 0x0000); break; // Catch invalid values
    }
    //serial_println(current_card);

    
}

button_state = false;

}



void Player2_turn(int hand[], int size) { //player 2's turn
    // Wait for a button press
    //turns the game on again
    if (GetBit(PINC, 5) == 0 ) {
        if(GetBit(PINC, 4) == 1 && select_pt2 == false ){//select button
            switch (hand[current_card_pt2]) {
            case 1: 
            serial_println("ACE_2");
            temp_array_pt2[counter_hand_pt2] = 1;
            //AMT_Selected++;
            
            
            break;
            case 2: 
            serial_println("KING_2");
            temp_array_pt2[counter_hand_pt2] =2;
            //AMT_Selected++;
          

            break;
            case 3: 
            
            serial_println("QUEEN_2");
            temp_array_pt2[counter_hand_pt2]=3;
            //AMT_Selected++;

             break;
            case 4: 
            serial_println("JOKER_2");
            temp_array_pt2[counter_hand_pt2] = 4;
           
             break;
            
            }
            select_pt2 = true;

            
        }
        if(GetBit(PINC, 4) == 0 && select_pt2 == true){
            select_pt2 =false;
            counter_hand_pt2++;
        }

        if(GetBit(PINC, 4) == 1 && time_held < 5){
            _delay_ms(200);
            time_held++;
            if(GetBit(PINC, 4) == 0){
                time_held=0;
            }
        }
        else if(time_held >= 5){
            time_held=0;
            counter_hand=0;
            temp_array_pt2[3] =0;

        }
        
      
    }

    // Button is pressed; proceed with the rest of the logic
    button_state_pt2 = true;
   
    // Increment and check bounds
    
button_pressed_pt3++;
// Reset button cycle when it reaches 6
if (button_pressed_pt3 == 6) {
    button_pressed_pt3 = 1;
}


// Loop to move the "_" character while the button is pressed

if(GetBit(PINC, 5) == 1 ) {
    //Erase previous character
    
    // DrawString(90, pixel_start_pt2, "\\", 0xFFFF, 0x0000);

    // // Update position
    // pixel_start_pt2 -= 10;
    

    // // Draw new character
    // DrawString(90, pixel_start_pt2, " ", 0xFFFF, 0x0000);

    // // Loop back to starting position if out of bounds
    // if(button_pressed_pt2 == 1){
    //   DrawString(90, 60, "\\", 0xFFFF, 0x0000);
    // }
    // if (pixel_start_pt2 <= 60) {
    //     pixel_start_pt2 = 110;
    // }

    
    current_card_pt2++;
    if (current_card_pt2 >= size) {
    current_card_pt2 = 0; // Loop back to the beginning
    }

     switch (hand[current_card_pt2]) {
        case 1: DrawString(122, 50, "SELECT[ACE\\\\", 0xFFFF, 0x0000);  break;
        case 2: DrawString(122, 50, "SELECT[KING\\\\", 0xFFFF, 0x0000); break;
        case 3: DrawString(122, 50, "SELECT[QUEEN", 0xFFFF, 0x0000); break;
        case 4: DrawString(122, 50, "SELECT[JOKER", 0xFFFF, 0x0000); break;
        //default: DrawString(122, 50, "INVALID", 0xFFFF, 0x0000); break; // Catch invalid values
    }
    //serial_println(current_card_pt2);

    
}

button_state_pt2 = false;

}



// Function to deal cards to players
void dealCards(int deck[], int *deckSize, int hand[], int *handSize) {
    *handSize = HAND_SIZE;
    for (int i = 0; i < HAND_SIZE; i++) {
        hand[i] = deck[0];

        // Remove the dealt card from the deck
        for (int j = 0; j < *deckSize - 1; j++) {
            deck[j] = deck[j + 1];
        }
        (*deckSize)--;
    }
}