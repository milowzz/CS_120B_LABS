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


int main(int argc, char *argv[]) {
  

    
    //output 1st argument
    cout << "1st Argument: " << atoi(argv[1]) << endl;


    //store the 1st argument
    unsigned char arg_one = atoi(argv[1]);
    
    //output the conversion pt 1
    cout << atoi(argv[1]) << " in binary: ";

    // Get the binary representation of the unsigned char
     char* binaryString = cptrCharToBinary(arg_one);

    //output the conversion pt 2
    cout << binaryString << endl;


    //free up the allocated memory used in the first function
    free(binaryString);

    return 0;
}
