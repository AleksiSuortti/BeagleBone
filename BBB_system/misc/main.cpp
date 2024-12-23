#include <iostream>
#include <thread>
#include <chrono>
#include "BBB_gpio.h"
#include "BBB_sys.h"
#include "httplib.h"
#include <json.hpp>

int main() {

    using json = nlohmann::json;

    BBB_sys::BBB_i2c_oled OLED(2);

    httplib::Server server;

    auto& display = *OLED.getDisplay();

    server.Post("/display", [&display, &OLED](const httplib::Request &req, httplib::Response &res) {
        
        // Parse the request body as JSON
        json reqJSON;
        try {
            reqJSON = json::parse(req.body);
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content("Invalid JSON format", "text/plain");
            return;
        }

        // Retrieve the "text" field from the parsed JSON
        std::string text = reqJSON.value("text", "");

        if (text.empty()) {
            res.status = 400;
            res.set_content("No text provided", "text/plain");
            return;
        }

        std::cout << "Received message: " << text << std::endl;

        std::vector<std::string> print;
        print.push_back(text);

        display.clearBuffer();
        OLED.textBox(print, 20, 20);
        display.renderDisplay(0, 7);

        res.set_content("Text displayed succesfully", "text/plain");
    });

    std::cout << "Server is running on http://0.0.0.0:5000" << std::endl;
    server.listen("0.0.0.0", 5000);

    return 0;
}