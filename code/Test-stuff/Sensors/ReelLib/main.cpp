// Comment: This is the Hello Pi Program.
#include <stdio.h>
#include <pigpio.h>
#include <chrono>
#include <thread>
#include <iostream>
#include "Reel.h"
using namespace std;


int main(){
    printf("Hello Pi\n\r");
    // Initialize the pigpio library
	Reel reel_dev(1);
	
	//reel_dev.definePosition(32);
	
	//reel_dev.releaseLatches();
	reel_dev.releaseLatch(Latch::Latch_A);
	
	
	//reel_dev.cutRope();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	reel_dev.releaseLatch(Latch::Latch_B);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	reel_dev.releaseLatch(Latch::Latch_C);
    
	
	while(1){
		char letter;
		int number;

		fprintf(stderr, "Current Position: %d\n\r", reel_dev.getPosition());

		// Read input
		cout << "Enter a letter: ";
		cin >> letter;
		cout << "Enter a number: ";
		cin >> number;

		// Perform switch based on the letter
		switch (letter) {
			case 'p': 	// Set position
				reel_dev.setPosition(number);
				break;
			case 's':
				reel_dev.setSpeed(number);
				break;
			case 'z':
				
				cout << "You entered letter C. Number divided by 2: " << number / 2.0 << "\n\r";
				break;
			default:
				cout << "Invalid letter. Please enter A, B, or C." << "\n\r";
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	
	/*
    // Open I2C device on bus 1, address 0x40 (change this to your device's address)
    int i2cHandle = i2cOpen(1, 0x02, 0);
    if (i2cHandle < 0) {
        fprintf(stderr, "Unable to open I2C device\n");
    } else {
        // Assuming your device expects a single byte command (0x01) followed by a value.
        // Replace these with commands suitable for your device.
        int command = I2C_SET_PWR; // Example command
        int value = 0;   // Example value to send
        char data[3];
        data[0] = I2C_SET_PWR;
        data[1] = (value>>8) & 0xFF;
        data[2] = value & 0xFF;

        // Write a command and a value to the device
        int result = i2cWriteDevice(i2cHandle, data, 3);
        
        if (result != 0) {
            fprintf(stderr, "Failed to write to the I2C device\n");
        }


        // Read a byte from the device, assuming it sends a response after the command.
        //int readValue = i2cReadByteData(i2cHandle, command);
        //printf("Read value: 0x%X\n", readValue);

        // Close the I2C device
        i2cClose(i2cHandle);
    }*/

    

    return 0; //This is also a comment and is ignored.
}// Comment: This is the Hello Pi Program.


