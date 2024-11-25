// SSD1306.h header file

#ifndef SSD1306_H
#define SSD1306_H

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <string>
#include <bitset>
#include <thread>
#include <chrono>
#include <cmath>

#define I2C_SLAVE_ADDR 0x3C

// Display configuration commands
#define DISP_OFF 0xAE
#define SET_DISP_CLK 0xD5
#define SET_MULTIPLEX 0xA8
#define SET_DISP_OFFSET 0xD3
#define SET_DISP_START_LN 0x40
#define SET_CHARGE_PUMP 0x8D
#define SET_MEM_ADD_MODE 0x20
#define SET_SEG_REMAP 0xA1
#define SET_COM_OUTPUT_SC_DIR 0xC8
#define SET_COM_PINS 0xDA
#define SET_DISP_CONTRAST 0x81
#define SET_PRE_CH_PRD 0xD9
#define SET_PX_TURNOFF_V 0xDB
#define DISP_FULL_PX_ON 0xA4
#define SET_DISP_NORM 0xA6
#define DISP_ON 0xAF

// Display configuration parameters
#define DISPCLK_DIV 0x80
#define MULTIPLEX_64 0x3F
#define DISP_OFFSET 0x00
#define ENABLE_CH_PUMP 0x14
#define HOR_MEM_ADD_MODE 0x00
#define CONFIG_COM_PINS 0x12
#define CONTRAST_LEVEL 0xCF
#define CONFIG_PRE_CH_PRD 0xF1
#define PX_TURNOFF_V 0x40
#define BLACK 0
#define WHITE 1

class SSD1306 {
    public:

        SSD1306(const int bus);

        ~SSD1306();

        bool begin();

        void initDisplay();
        
        void reset();

        void setDisplayState(bool is_on);

        void clearBuffer();

        void clearDisplay();

        // This is for debugging use
        void pageTest();

        int setCursor(uint8_t col, uint8_t page);

        int sendCommand(const uint8_t *commands, size_t len);

        void drawText(const std::string& text, int x, int y);

        void drawChar(char c, int x, int y);

        void draw_8(uint8_t* bitmap, size_t width, int x, int y);

        void drawPixel(int x, int y, int color);

        void drawLine(int x0, int y0, int x1, int y1, int width, int color);

        void drawHrzLine(int x0, int x1, int y, int color);

        void drawVertLine(int y0, int y1, int x, int color);

        void drawRectangle(int x0, int y0, int x1, int y1, int color);

        void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int width, int color);

        void drawEqTriangle(int tipX, int tipY, int height, int width, int color);

        void draw_64(uint64_t* bitmap);

        void startHorizontalScroll(int startPage, int endPage, int direction, int speed);

        // Diagonal scrolling wip does not want to cooperate
        void startDiagonalRightScrl(int startPage, int endPage);

        void startDiagonalLeftScrl(int startPage, int endPage);

        void stopScroll();

        void renderDisplay();

        uint8_t* ASCIImap(char c);

    private:
        const int bus;                // The i2c bus number
        int file;               // File descriptor for i2c communication
        uint8_t cursor[3];            // Page cursor
        uint64_t frameBuffer[128];    // Buffer for current display frame
};

#endif // SSD1306_H
