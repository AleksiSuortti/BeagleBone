/*
SSD1306 display test program
*/
#include <iostream>
#include "SSD1306.h"

int main() {

    SSD1306 display(2);
    if(!display.begin()) {
        std::cerr << "Failed to initialize display\n";
        return 1;
    }

    display.pageTest();
    
    uint8_t* charPtr = display.ASCIImap('a');
    
    std::cout << "Does this work? " << std::bitset<8>(*charPtr) << ", " << std::bitset<8>(*charPtr & 0xFF) << std::endl;

    for(int i = 0; i < 8; i++) {
        std::cout << std::bitset<8>(*(charPtr + i)) << std::endl;
    }

    return 0;
}