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

class SSD1306 {
    public:

        /*constructor that accepts the i2c bus number of the beaglebone eg. I2C-1
          there is an i2c-0 bus but that is typically reserved for HDMI
        */
        SSD1306(const int bus);

        // Desctructor to close the file descriptor
        ~SSD1306();

        // Initialize the SSD1306 display
        bool begin();

        // Give the display initial configuration
        void initDisplay();
        
        // Clear the display
        void clearDisplay();

        void pageTest();

        // Sets the cursor position for the nect write operation
        void setCursor(uint8_t col, uint8_t page);

        // Sends a sequence of commands to the SSD1306
        void sendCommand(const uint8_t *commands, size_t len);

        // Method to draw text on the display
        void drawText(const std::string& text, int x, int y);

        // Method for drawing characters
        void drawChar(char c, int x, int y);

        // Returns a pointer to "6x8" map for an ASCII character
        const uint8_t* ASCIImap(char c);

    private:
        const int bus; // The i2c bus number
        int file;
        uint8_t cursor[3];      // File descriptor for i2c communication
};

#endif // SSD1306_H
