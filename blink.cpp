
/* On-board LED flashing program
 * 	This program uses USR LED 0 and has three functions
 * 	LED on
 * 	LED off
 * 	LED flash
 * 	LED status
 */

#include<iostream>
#include<fstream>
#include<string>

#define LED0_PATH "/sys/class/leds/beaglebone:green:usr0"

// remove the trigger from the LED
void removeTrigger() {
	std::fstream fs;
	fs.open(LED0_PATH "/trigger", std::fstream::out);
	fs << "none";
	fs.close();
}

int main(int argc, char* argv[]) {

	// Message on details of usage
	if(argc != 2) {
		std::cout << "Usage is LEDstate: [parameter]" << std::endl;
		std::cout << "[parameter] options are:" << std::endl;
		std::cout << "on\noff\nflash\nstatus" << std::endl;
	}
	
	enum param {
		off = 0,
		on = 1,
		flash = 2,
		status = 3
	};
	
	int input = std::atoi(argv[1]);
	
	if(input < off || input > status) {
		std::cerr << "Invalid value. Must be between 0 and 3." << std::endl;
		return 1;
	}
	
	enum param cmd = static_cast<param>(input);
	
	std::fstream fs;
	std::cout << "Starting the LED flashing program" << std::endl;
	std::cout << "The LED path is: " << LED0_PATH << std::endl;
	
	// select action
	switch(cmd) {
		case 0:
		
			// Set led state to off
			removeTrigger();
			
			// Using the brigthness setting to turn LED off
			fs.open (LED0_PATH "/brightness", std::fstream::out);
			fs << "0";
			fs.close();
			break;
		case 1:
		
			removeTrigger();
			fs.open (LED0_PATH "/brightness", std::fstream::out);
			fs << "1";
			fs.close();
			break;
		case 2:
		
			removeTrigger();
			fs.open (LED0_PATH "/trigger", std::fstream::out);
			fs << "timer";
			fs.close();
			fs.open (LED0_PATH "/delay_on", std::fstream::out);
			fs << "250";
			fs.close();
			fs.open (LED0_PATH "/delay_off", std::fstream::out);
			fs << "250";
			fs.close();
			break;
		case 3:
		
			fs.open (LED0_PATH "/trigger", std::fstream::in);
			std::string line;
			while(getline(fs, line)) std::cout << line;
			fs.close();
			break;
	}
	std::cout << "Finishedthe LED flashing program" << std::endl;
	return 0;
}