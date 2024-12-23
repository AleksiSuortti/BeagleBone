#ifndef BBB_SYS_H
#define BBB_SYS_H

#include <vector>
#include <cmath>
#include <algorithm>
#include "SSD1306.h"
#include <iostream>
#include <thread>
#include <chrono>
#include "BBB_gpio.h"
#include "httplib.h"
#include <json.hpp>
#include <iomanip>
#include <cstdlib>

namespace BBB_sys {

    int mapRange(int value, int range2_min, int range2_max, int range1_min, int range1_max);

    std::string getCurrentTime();

    bool hasInternetConnection();

    class BBB_i2c_oled {

        public:
            BBB_i2c_oled(int bus);

            ~BBB_i2c_oled();

            void init();

            void textBox(const std::vector<std::string>& text, int x, int y);

            void textBox(const std::string& text, int x, int y);

            void progressBarHrz(int x1, int y1, int x2, int y2, int color, int min, int max, int value);

            void progressBarVrt(int x1, int y1, int x2, int y2, int color, int min, int max, int value);

            void updateScreen();

            SSD1306* getDisplay();
            
        private:
            SSD1306 display;
    };

    class Menu {

        public:

            Menu(int x, int y, std::vector<std::string> elements, BBB_i2c_oled& OLED);

            ~Menu();

            void updateMenu(int x, int y);

            void drawMenu(int x, int y);

            void scrollDown();

            void scrollUp();

            int getActiveElement();

        private:
            const std::vector<std::string> elements;
            int activeElement;
            BBB_i2c_oled& OLED;
    };

    class System {
        public:
            System(int i2c_bus);
            ~System();

            void init_frame();

            void start_HTTP_server();
            
            void run();

            void updateState();

            void checkMessages();

            void emptyMessages();

            std::vector<std::string>& getHTTPmessages();

            BBB_i2c_oled& getOLED();

            Menu& getMain_menu();

        private:
            BBB_i2c_oled OLED;
            Menu main_menu;
            std::vector<std::string> HTTPmessages;
            std::mutex messages_mutex;
    };

};

#endif // BBB_SYS_H