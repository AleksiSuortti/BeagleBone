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
    
    return 0;
}