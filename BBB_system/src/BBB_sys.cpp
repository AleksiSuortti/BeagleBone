#include "BBB_sys.h"

namespace BBB_sys {

    int mapRange(int value, int range2_min, int range2_max, int range1_min, int range1_max) {
        return range1_min + (value - range2_min) * (range1_max - range1_min) / (range2_max - range2_min);
    }

    std::string getCurrentTime() {

        auto now = std::chrono::system_clock::now();

        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm = *std::localtime(&now_c);

        std::ostringstream oss;

        oss << std::put_time(&local_tm, "%H:%M");

        return oss.str();
    }

    bool hasInternetConnection() {
        int connection_check = system("ping -c 1 -W 1 google.com > /dev/null 2>&1");
        return connection_check == 0;
    }

    BBB_i2c_oled::BBB_i2c_oled(const int bus) : display(bus) {

        if(!this->display.begin()) {
            std::cerr << "Failed to initialize display\n";
        }
        init();
    }

    BBB_i2c_oled::~BBB_i2c_oled() {
        this->display.~SSD1306();
        std::cout << "Display object deleted" << std::endl;
    }

    void BBB_i2c_oled::init() {

        this->display.clearBuffer();
        this->display.drawCircle(62, 30, 15, WHITE);
        this->display.renderDisplay(0,7);
        this->display.fillCircle(70, 30, 15, WHITE);
        this->display.renderDisplay(0,7);
        this->display.drawCircle(75, 30, 15, BLACK);
        this->display.renderDisplay(0,7);
        this->display.inverseDisplay(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(750));
        this->display.inverseDisplay(false);
        this->display.clearBuffer();
        this->display.renderDisplay(0,7);
    }

    void BBB_i2c_oled::textBox(const std::vector<std::string>& text, int x, int y) {

        size_t maxLineLength = 0;
        int rowIndex = 0;
        
        for(const std::string& row : text) {

            maxLineLength = std::max(maxLineLength, row.length());

            this->display.drawText(row, x + 2, y + (rowIndex * 8) + 2);

            rowIndex++;
        }

        int y_end = y + text.size() * 8 + 4;
        int x_end = x + maxLineLength * 6 + 2;
        this->display.drawRectangle(x, y, x_end, y_end, WHITE);
    }

    void BBB_i2c_oled::textBox(const std::string& text, int x, int y) {

        int y_end = y + 12;
        int x_end = x + (text.length() * 6) + 2;

        this->display.drawText(text, x + 2, y + 2);
        this->display.drawRectangle(x, y, x_end, y_end, WHITE);
    }

    void BBB_i2c_oled::progressBarHrz(int x1, int y1, int x2, int y2, int color, int min, int max, int value) {

        if(value >= min && value <= max) {
                
            this->display.drawRectangle(x1, y1, x2, y2, color);

            int progressEndPoint = mapRange(value, min, max, x1 + 2, x2 + 2);

            this->display.fillRectangle(x1 + 2, y1 + 2, progressEndPoint, y2 - 2, color);
        }
    }

    void BBB_i2c_oled::progressBarVrt(int x1, int y1, int x2, int y2, int color, int min, int max, int value) {

        if(value >= min && value <= max) {
                
            this->display.drawRectangle(x1, y1, x2, y2, color);

            int progressEndPoint = mapRange(value, min, max, y1 + 2, y2 + 2);

            this->display.fillRectangle(x1 + 2, y1 + 2, x2 - 2, progressEndPoint, color);
        }
    }

    SSD1306* BBB_i2c_oled::getDisplay() {

        return &this->display;
    }

    void BBB_i2c_oled::updateScreen() {
        this->display.renderDisplay(0,7);
        this->display.clearBuffer();
    }

    Menu::Menu(int x, int y, std::vector<std::string> elements, BBB_i2c_oled& OLED) : elements(std::move(elements)), activeElement(0), OLED(OLED) {
        drawMenu(x, y);
    }

    Menu::~Menu() {
    };

    void Menu::drawMenu(int x, int y) {

        int rows = floor((63 - y)/14);
        int columns = (int)ceil((float)this->elements.size()/(float)rows);

        if((127 - x < columns * 30)) {
            std::cerr << "Too many menu elements to display" << std::endl;
            return;
        }

        int index = 0;

        for(int colInd = 0; colInd < columns; colInd++) {
            for(int rowInd = 0; rowInd < rows; rowInd++) {
                
                int elem_coord_x = x + (colInd * 30);
                int elem_coord_y = y + (rowInd * 14);

                if(index < this->elements.size()) {
                    if(index == this->activeElement) {
                        this->OLED.getDisplay()->drawRectangle(elem_coord_x - 1, elem_coord_y - 1, elem_coord_x + 27, elem_coord_y + 13, WHITE);
                    }
                    this->OLED.textBox(this->elements[index], elem_coord_x, elem_coord_y);
                }
                index++;
            }
        }
    }

    void Menu::updateMenu(int x, int y) {
        drawMenu(x, y);
    }

    void Menu::scrollDown() {

        if(this->activeElement < this->elements.size() - 1) {
            this->activeElement++;
        }
        else if(this->activeElement == this->elements.size() - 1) {
            this->activeElement = 0;
        }
        else {
            std::cerr << "Active menu element index error" << std::endl;
        }
        return;
    }

    void Menu::scrollUp() {

        if(this->activeElement > 0) {
            this->activeElement--;
        }
        else if(this->activeElement == 0) {
            this->activeElement = this->elements.size() - 1;
        }
        else {
            std::cerr << "Active menu element inder error" << std::endl;
        }
        return;
    }

    int Menu::getActiveElement() {
        return this->activeElement;
    }

    System::System(int i2c_bus) : OLED(i2c_bus), HTTPmessages(), main_menu(5,17,{"MESG", "STAT", "ITM1", "....", "....", "...."}, this->OLED) {
    }

    System::~System() {
        this->OLED.~BBB_i2c_oled();
    }

    void System::init_frame() {
        
        this->OLED.getDisplay()->drawRectangle(0,0,127,63,WHITE);
        this->OLED.getDisplay()->drawText("BEAGLE sys",2,2);
        this->OLED.getDisplay()->drawText(getCurrentTime(), 70, 2);

        if(hasInternetConnection()) {

            uint8_t connectedSymbol[] = {
                0b00000000,
                0b11100000,
                0b00000000,
                0b11111000,
                0b11111000,
                0b00000000,
                0b11111111,
                0b11111111
            };
            this->OLED.getDisplay()->draw_8(connectedSymbol, 8, 102, 2);
        }
        else {

            uint8_t connectedSymbol[] = {
                0b00000000,
                0b00000101,
                0b11000010,
                0b00000101,
                0b11110000,
                0b00000000,
                0b11111100,
                0b00000000

            };
            this->OLED.getDisplay()->draw_8(connectedSymbol, 8, 102, 2);
        }
        
        this->OLED.progressBarVrt(115, 5, 120, 58, WHITE, 0, 163, BBB_gpio::analogRead(0) * 100);
    }

    void System::start_HTTP_server() {

        using json = nlohmann::json;

        httplib::Server server;

        server.Post("/display", [this](const httplib::Request &req, httplib::Response &res) {

            json reqJSON;

            try {
                reqJSON = json::parse(req.body);
            } catch (const std::exception& e) {
                res.status = 400;
                res.set_content("Invalid JSON foramt", "text/plain");
                return;
            }

            std::string text = reqJSON.value("text","");

            if(text.empty()) {
                res.status = 400;
                res.set_content("No text provided", "text/plain");
                return;
            }

            // Lock the mutex before modifyign shared data
            std::lock_guard<std::mutex> lock(messages_mutex);
            this->HTTPmessages.push_back(text);

            res.set_content("Text received succesfully", "text/plain");
        });

        std::cout << "Server is running on http://0.0.0.0:5000" << std::endl;
        server.listen("0.0.0.0", 5000);
    }

    void System::run() {
        
        std::thread HTTP_server_thread(&System::start_HTTP_server, this);

        HTTP_server_thread.detach();
    }

    std::vector<std::string>& System::getHTTPmessages() {

        // Lock the mutex before accessing shared data
        std::lock_guard<std::mutex> lock(messages_mutex);
        return this->HTTPmessages;
    }

    BBB_i2c_oled& System::getOLED() {
        return this->OLED;
    }

    Menu& System::getMain_menu() {
        return this->main_menu;
    }

    void System::updateState() {

        init_frame();
        this->OLED.updateScreen();
    }

    void System::checkMessages() {

        this->OLED.getDisplay()->clearBuffer();

        std::cout << "CheckMessages" << std::endl;

        if(this->HTTPmessages.empty()) {
            std::cerr << "No messages in queue" << std::endl;
            return;
        }

        int messageIndex = 0;
        bool leftBtn_press = false;

        Menu message_options(5, 45, {"EXIT", "DELT"}, this->OLED);

        while(true) {

            std::cout << "In the check message loop" << std::endl;

            init_frame();
            message_options.drawMenu(5, 45);
            this->OLED.textBox(this->HTTPmessages[messageIndex], 5, 20);

            leftBtn_press = (BBB_gpio::digitalRead(67) == 1) ? true : false;
            
            if(leftBtn_press) {
                messageIndex++;
            }
            if(messageIndex == this->HTTPmessages.size() - 1) {
                break;
            }
            this->OLED.updateScreen();
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
        std::cout << "End of messages" << std::endl;

        this->OLED.getDisplay()->clearBuffer();
    }
}