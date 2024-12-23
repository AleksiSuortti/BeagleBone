#include "BBB_sys.h"

#define DIGI_PIN1 66
#define DIGI_PIN2 67
#define DIGI_PIN3 69
#define ANALOG_PIN 0

int main() {

    using namespace BBB_gpio;

    BBB_sys::System system(2);

    system.init_frame();
    system.getOLED().updateScreen();
    system.run();

    pinMode(DIGI_PIN1, "in");
    pinMode(DIGI_PIN2, "in");
    pinMode(DIGI_PIN3, "out");
    // pinMode(ANALOG_PIN, "in");

    /*
    while(true) {
        std::vector<std::string> messages = system.getHTTPmessages();

        for(const auto& msg : messages) {
            system.getOLED().getDisplay()->clearBuffer();
            system.getOLED().getDisplay()->drawText(msg, 5, 5);
            system.getOLED().getDisplay()->renderDisplay(0,7);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    */

    int analog_control = 0;
    int analog_temp = 0;
    bool startup = true;
    bool rightBtn_press = false;
    bool leftBtn_press = false;

    while(true) {

        rightBtn_press = (digitalRead(66) == 1) ? true : false;
        leftBtn_press = (digitalRead(67) == 1) ? true : false;

        if(!startup) {

            analog_control = analogRead(ANALOG_PIN) * 100;

            if(analog_temp - analog_control >= 2) {
                system.getMain_menu().scrollUp();
            } else if(analog_control - analog_temp >= 2 ) {
                system.getMain_menu().scrollDown();
            }
        }

        system.getMain_menu().updateMenu(5, 20);

        analog_temp = analog_control;

        startup = false;

        system.updateState();



        if(rightBtn_press) {

            digitalWrite(DIGI_PIN3, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            digitalWrite(DIGI_PIN3, 0);

            switch(system.getMain_menu().getActiveElement()) {
                case 0:

                    std::cout << "Message function" << std::endl;
                    rightBtn_press = false;
                    leftBtn_press = false;

                    system.checkMessages();
                    break;
                case 1:

                    break;
                case 2:
                    
                    break;
                case 3:

                    break;

                default:
                    std::cout << "Selected menu item has no defined action" << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(25));
            
    }

    return 0;
}