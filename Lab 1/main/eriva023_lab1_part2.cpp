/*         Name: Emilio Rivas
           Email: eriva023@ucr.edu

*          Discussion Section: 024

 *         Assignment: Lab #1  Exercise #3

 *         Exercise Description: [optional - include for your own benefit]

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *         Demo Link: <https://youtu.be/Dbmqu4t-Yhc>

 */

#include <iostream>
#include <cstdlib> 
#include <string>
using namespace std;

// Function to convert an unsigned char to a binary string
char* cptrCharToBinary(const unsigned char arg_one) {
    unsigned char copy_arg_one = arg_one;
     char* binary = (char*)malloc(sizeof(char[9]));
    binary[8] = '\0'; // Null terminator

    for (int i = 7; i >= 0; --i) {
        binary[i] = (copy_arg_one % 2) + '0'; // 48 in ASCII is '0', so it is either 48 + 0 = '0' 0r 48 + 1 = '1'
        copy_arg_one /= 2; // Shift bits right
    }

    return binary;
}

char cCharHexSelector(const unsigned char arg_one, const unsigned char arg_two){

   char* binary_func_two = cptrCharToBinary(arg_one);
   int result = 0;
   int result_case2 = 0;
   char lower; //lower return value
   char upper; //upper return value
   int counter = 8; // used to convert binary into hex, by multipling by 8,4,2,1. Then stores this value into a result variable 
  


 switch (arg_two) {
    //LOWER if the second argument is zero (select lower 4 bits)
        case 0: 
            for (int i = 4; i <= 7; i++) {
                if(binary_func_two[i] % 2 == 0){
                    result+=  counter * 0;
                }
                else if(binary_func_two[i] % 2 == 1){
                    result+=  counter * 1;
                }
                 if(counter == 0){
                    counter = 8;
                    }
                        else{
                        counter /= 2;
                        }
            }
            
            
            cout << "lower 4 bits of " << static_cast<int>(arg_one) << ": ";
            if(result < 10){
                 lower = result + '0'; //ASCII char's numbers 0-9
            }
            else{
                lower = result + '7';//ASCII char's letter A-F
            }
           
            return lower;
    //UPPER  if the second argument is one (select upper 4 bits)
        case 1:
            for (int j = 0; j <= 3; j++) { //iterate through the upper bits
                if(binary_func_two[j] % 2 == 0){ 
                    result_case2+= counter * 0;
                }
                else if(binary_func_two[j] % 2 == 1){
                    result_case2+= counter * 1;
                }
                
                    if(counter == 0){
                    counter = 8;
                    }
                        else{
                        counter /= 2;
                        }
           
                
            }
        
            cout << "upper 4 bits of " << static_cast<int>(arg_one) << ": ";
             if(result_case2 < 10){
                 upper = result_case2 + '0'; //ASCII char's numbers 0-9 (number 48)
            }
            else{
                upper = result_case2 + '7';//ASCII char's letter A-F (53 I think?)
            }
           
            return upper;

        default:
            cout << "Invalid option. Please select a number between 0 and 1." << endl;
            return 0;
}

}
    



int main(int argc, char *argv[]) {
  if (argc != 3) { // Expecting exactly 2 arguments (put 5 in if statement for four arguments)
        cout << "Insufficient amount of arguments " << argv[0] << endl;
        return 1;
    }

    
    //output 1st argument
    cout << "1st Argument: " << atoi(argv[1]) << endl;

    
    //store the 1st argument
    unsigned char arg_one = atoi(argv[1]);
    
    //output the conversion pt 1
    cout << atoi(argv[1]) << " in binary: ";

    //store the 2nd argument
     unsigned char arg_two = atoi(argv[2]);

    // Get the binary representation of the unsigned char
     char* binaryString = cptrCharToBinary(arg_one);

    //output the conversion pt 2
    cout << binaryString << endl;

    //now we chnage the char* into an int and store into a unsigned char again for the second part
   
    //call the second function
    char Hexchar = cCharHexSelector(arg_one, arg_two);
    cout << Hexchar << endl;

    //free up the allocated memory used in the first 
    free(binaryString);

    return 0;
}
