/*
SSD1306 display test program
*/
#include <iostream>
#include "SSD1306.h"
#include <thread>
#include <chrono>

int main() {

    SSD1306 display(2);
    if(!display.begin()) {
        std::cerr << "Failed to initialize display\n";
        return 1;
    }
    
    for(int i = 0; i < 64; i += 8) {

            display.draw_8(display.ASCIImap(static_cast<char>(0xFF)), 8, 2*i, i);
    }

    display.renderDisplay(0,7);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    display.clearDisplay();

    for(int i = 0; i < 5; i++) {
        display.clearBuffer();
        display.drawRectangle(i,i,i+30,i+30, WHITE);
        display.drawRectangle(2*i,2*i,2*i+30,2*i+30, WHITE);
        display.drawText("miumau ^(>.<)^", i, i);
        display.draw_8(display.ASCIImap(static_cast<char>(0xFF)), 8, 2*i, i);
        display.renderDisplay(0,7);
    }

    display.clearBuffer();

    display.drawTriangle(30, 10, 10, 50, 70, 45, 5, WHITE);

    for(int i = 0; i < 10; i++) {

        int color = (i % 2 == 0) ? WHITE : BLACK;

        display.drawEqTriangle(50 + (3*i), 5 + i, 20 + 2*i, 3, color);

        display.renderDisplay(0,7);
    }

    for(int i = 9; i >= 0; i--) {
        
        int color = (i % 2 == 0) ? BLACK : WHITE;

        display.drawEqTriangle(50 + (3*i), 5 + i, 20 + 2*i, 3, color);

        display.renderDisplay(0,7);
    }

    display.clearBuffer();
    display.drawCircle(62, 30, 15, WHITE);
    display.renderDisplay(0,7);
    display.fillCircle(70, 30, 15, WHITE);
    display.renderDisplay(0,7);
    display.drawCircle(75, 30, 15, BLACK);
    display.renderDisplay(0,7);
    display.inverseDisplay(true);

    display.clearDisplay();

    for(int i = 0; i < 50; i++) {
        display.clearBuffer();
        display.drawText("Softa liuku !#¤%", 10 + 2*i, 10 + i);
        display.renderDisplay(0,7);
        
    }
    display.inverseDisplay(false);
    return 0;
}