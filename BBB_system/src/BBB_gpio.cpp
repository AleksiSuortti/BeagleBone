#include <iostream>
#include <fstream>
#include <string>
#include "BBB_gpio.h"

void BBB_gpio::pinMode(int pin, const std::string& mode) {

    std::string direction_path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/direction";
    std::ofstream directionFile(direction_path);

    if(!directionFile.is_open()) {
        std::cerr << "Failed to open export file" << std::endl;
        return;
    }

    if(mode == "out" || mode =="in") {
        directionFile << mode;
        std::cout << "GPIO" << pin << " set as " << mode << std::endl;
    }
    else {
        std::cerr << "Invalid mode: " << mode << ". Use 'in' or 'out'." << std::endl;
    }
    directionFile.close();
}

float BBB_gpio::analogRead(int analog_channel) {

    std::string adcPath = "/sys/bus/iio/devices/iio:device0/in_voltage" + std::to_string(analog_channel) + "_raw";
    std::ifstream adcFile(adcPath);

    if(!adcFile.is_open()) {
        std::cerr << "Failed to open ADC device file for channel " << analog_channel << std::endl;
        return -1.0f;
    }

    int rawValue;
    adcFile >> rawValue;
    adcFile.close();

    // Conversion from adc reading to voltage [0 : 1.8] V
    return rawValue * (1.8/4095.0);
}

int BBB_gpio::digitalRead(int digital_pin) {

    std::string digiGPIO_path = "/sys/class/gpio/gpio" + std::to_string(digital_pin) + "/value";
    std::ifstream digitalFile(digiGPIO_path);

    if(!digitalFile.is_open()) {
        std::cerr << "Failed to open GPIO" << digital_pin << " device file" << std::endl;
        return -1;
    }

    int digiValue;
    digitalFile >> digiValue;
    digitalFile.close();
    return digiValue;
}

void BBB_gpio::digitalWrite(int digital_pin, int value) {

    if(value == 1 || value == 0) {
        std::string digiGPIO_path = "/sys/class/gpio/gpio" + std::to_string(digital_pin) + "/value";
        std::ofstream digitalFile(digiGPIO_path);

        if(!digitalFile.is_open()) {
            std::cerr << "Failed to open GPIO" << digital_pin << " device file" << std::endl;
            return;
        }

        digitalFile.write(std::to_string(value).c_str(),1);
        std::cout << "value " << value << "written to pin " << digital_pin << std::endl;
        digitalFile.close();
    }
    return;
}