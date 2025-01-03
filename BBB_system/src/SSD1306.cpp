/*
    SSD1306.cpp

*/
#include "SSD1306.h"

// Constructor to initialize the i2c bus
SSD1306::SSD1306(const int bus) : bus(bus), file(-1), cursor{0,0,0} {}

// Destructor to close the file descriptor if open
SSD1306::~SSD1306() {
    if (this->file >= 0) {
        close(this->file);
    }
}

// Initialize the SSD1306 display
bool SSD1306::begin() {

    // Open the device file of specified i2c bus
    char file_path[20];
    snprintf(file_path, 19, "/dev/i2c-%d", bus);
    this->file = open(file_path, O_RDWR);

    if(this->file < 0) {
        std::cerr << "Failed to open the i2c bus" << std::endl;
        return false;
    }

    std::cout << "File opened: " << file << std::endl;
    
    // Tell i2c driver the file descriptor is used to communicate with the device at I2C_SLAVE_ADDR
    if(ioctl(this->file, I2C_SLAVE, I2C_SLAVE_ADDR) < 0) {
    	std::cerr << "Failed to open /dev/i2c-" << bus << std::endl;
	close(this->file);
    }

    initDisplay();
    clearDisplay();
    return true;
}

// Initialize the SSD1306 display with standard configuration
void SSD1306::initDisplay() {
    static const uint8_t init_sequence[] = {
        DISP_OFF,
        SET_DISP_CLK, DISPCLK_DIV,
        SET_MULTIPLEX, MULTIPLEX_64,
        SET_DISP_OFFSET, DISP_OFFSET,
        SET_DISP_START_LN,
        SET_CHARGE_PUMP, ENABLE_CH_PUMP,
        SET_MEM_ADD_MODE, HOR_MEM_ADD_MODE,     // Set memory addressing mode to horizontal
        SET_SEG_REMAP,           // Set segment re-map 0 to 127
        SET_COM_OUTPUT_SC_DIR,           // Set COM output scan direction
        SET_COM_PINS, CONFIG_COM_PINS,     // Set Com pins hardware configuration
        SET_DISP_CONTRAST, CONTRAST_LEVEL,     // Set contrast control
        SET_PRE_CH_PRD, CONFIG_PRE_CH_PRD,     // Set pre-charge period
        SET_PX_TURNOFF_V, PX_TURNOFF_V,     // Set VCOMh Deselect level
        DISP_FULL_PX_ON,           // Disable entire display on (0xA5 for all on)
        SET_DISP_NORM,           // Set normal display (0xA7 for inverse)
        DISP_ON            // Display on
    };

    int init_bytes = sendCommand(init_sequence, sizeof(init_sequence));

    if(init_bytes != (sizeof(init_sequence) + 1)) {
        std::cerr << "Display init failed: (" << init_bytes << ") " << strerror(errno) << " while writing" << std::endl;
    };
}

void SSD1306::setDisplayState(bool is_on) {
    const uint8_t command = is_on ? 0xAF : 0xAE;
    sendCommand(&command, 1);
}

void SSD1306::reset() {
    initDisplay();
    clearDisplay();
}

void SSD1306::clearBuffer() {

    std::memset(frameBuffer, 0x00, sizeof(frameBuffer));
}

void SSD1306::clearDisplay() {

    clearBuffer();
    renderDisplay(0,7);
}

// Sets the cursor position for the next write operation
int SSD1306::setCursor(uint8_t col, uint8_t page) {

        this->cursor[0] = static_cast<uint8_t>(0xB0 + page);                 // Set page start address
        this->cursor[1] = static_cast<uint8_t>(0x00 + (col & 0x0F));          // Lower column start address
        this->cursor[2] = static_cast<uint8_t>(0x10 + ((col >> 4 ) & 0x0F));  // Higher column start addres
    
    return write(file, cursor, sizeof(cursor));
}

// Send commands to the display
int SSD1306::sendCommand(const uint8_t *commands, size_t len) {

    uint8_t com_buffer[len + 1];                    // +1 for the control byte

    com_buffer[0] = 0x00;                           // 0x00 is the control byte for commands

    std::memcpy(com_buffer + 1, commands, len);     // std::memcpy to copy the commands into the buffer after the control byte

    int debug = write(file, com_buffer, len + 1);   // Write the buffer to the i2c bus

    if(debug != (len + 1)) {
        std::cerr << "Failed to send i2c command:\n" <<
        debug << " out of " << len + 1 << " bytes written succesfully.\n" <<
        "Error: (" << errno << ") " << strerror(errno) << std::endl;
        return errno;
    }
    return debug;
}

void SSD1306::inverseDisplay(bool is_inverse) {
    const uint8_t command = is_inverse ?  SET_DISP_INVERSE : SET_DISP_NORM;
    sendCommand(&command, 1);
}

void SSD1306::drawText(const std::string& text, int x, int y) {

    // check if cursor coordinates are valid
    if((x < 0 || x > 127) || (y < 0 || y > 55)) {
        std::cout << "Cursor index error" << std::endl;
    }
    else {

        int x_cursor = x;

        for(char c : text){
            uint8_t charMap[8];    
            uint8_t* charMapPtr = ASCIImap(c);

            for(int i = 0; i < 8; i++) {
                charMap[i] = *(charMapPtr + i);    
            }
            draw_8(charMap, 8, x_cursor, y);
            x_cursor += 6;
        }
    }
}

void SSD1306::draw_8(uint8_t* bitmap, size_t width, int x, int y) {

    int x_end = x + static_cast<int>(width);
    int bitmap_ind = 0;
    if((x < 0 || x > 127) || (y < 0 && y > 55)) {
        std::cout << "Draw_8: Cursor index error" << std::endl;
    }
    else {
        for(int col = x; col < x_end; col++) {
            if(col < 128) {
                frameBuffer[col] |= ((static_cast<uint64_t>(bitmap[bitmap_ind])) << y);
                bitmap_ind++;
            }
        }
    }
}

void SSD1306::drawPixel(int x, int y, int color) {

    if((x < 0 || x > 127) || (y < 0 || y > 63)) {
        return;
    }
    color ? (this->frameBuffer[x] |= (1ULL << y)) : (this->frameBuffer[x] &= ~(1ULL << y));
}

// Usign Bresenham's line algorithm
void SSD1306::drawLine(int x0, int y0, int x1, int y1, int width, int color) {
    bool steep = abs(y1 - y0) > abs(x1 - x0); // Check if the line is steep
    if (steep) {
        // Swap x and y for steep lines
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        // Ensure we're always drawing from left to right
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = dx / 2;
    int yStep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        // Draw the pixel, reversing the swap for steep lines
        if (steep) {
            for (int i = 0; i < width; i++) {
                drawPixel(y + i, x, color); // Swap back x and y
            }
        } else {
            for (int i = 0; i < width; i++) {
                drawPixel(x, y + i, color);
            }
        }

        error -= dy;
        if (error < 0) {
            y += yStep;
            error += dx;
        }
    }
}

void SSD1306::drawHrzLine(int x0, int x1, int y, int color) {

    if (y < 0 || y > 63) return;

    int start_x = x0, end_x = x1;

    if(x0 > x1) {
        start_x = x1;
        end_x = x0;
    }
    for(int i = start_x; i <= end_x; i++) {
        drawPixel(i, y, color);
    }
}

void SSD1306::drawVertLine(int y0, int y1, int x, int color) {

    if (x < 0 || x > 127) return;

    int start_y = y0, end_y = y1;

    if(y0 > y1) {
        start_y = y1;
        end_y = y0;
    }
    for(int i = start_y; i <= end_y; i++) {
        drawPixel(x, i, color);
    }
}

void SSD1306::drawRectangle(int x0, int y0, int x1, int y1, int color) {
    drawHrzLine(x0, x1, y0, color);
    drawHrzLine(x0, x1, y1, color);
    drawVertLine(y0, y1, x0, color);
    drawVertLine(y0, y1, x1, color);
}

void SSD1306::fillRectangle(int x0, int y0, int x1, int y1, int color) {
    
    for(int i = y0; i <= y1; i++) {
        drawHrzLine(x0, x1, i, color);
    }
}

void SSD1306::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int width, int color) {

drawLine(x0, y0, x1, y1, width, color);
drawLine(x0, y0, x2, y2, width, color);
drawLine(x1, y1, x2, y2, width, color);
}

void SSD1306::drawEqTriangle(int tipX, int tipY, int height, int width, int color) {

    int left_x = tipX - floor((1732*height/2000));

    int right_x = tipX + floor((1732*height/2000));

    int base_y = tipY + height;

    drawLine(tipX, tipY, left_x, base_y, width, color);
    drawLine(tipX, tipY, right_x, base_y, width, color);
    drawLine(left_x, base_y, right_x, base_y, width, color);
}

void SSD1306::drawCircle(int x, int y, int radius, int color) {

    float dp;
    int x1,y1;
    x1 = 0;
    y1 = radius;
    dp = 3 - 2*radius;
    while(x1<=y1)
    {
        if(dp<=0)
            dp += (4 * x1) + 6;
        else
        {
            dp += 4*(x1-y1)+10;
            y1--;
        }
        x1++;
        drawPixel(x1+x, y1+y, color);
        drawPixel(x1+x, y-y1, color);
        drawPixel(x-x1, y1+y, color);
        drawPixel(x-x1, y-y1, color);
        drawPixel(x+y1, y+x1, color);
        drawPixel(x+y1, y-x1, color);
        drawPixel(x-y1, y+x1, color);
        drawPixel(x-y1, y-x1, color);
    }
}

void SSD1306::fillCircle(int x, int y, int radius, int color) {
    int dp;
    int x1 = 0;
    int y1 = radius;
    dp = 3 - 2 * radius;

    while (x1 <= y1) {
        // Fill spans directly
        for (int i = x - x1; i <= x + x1; i++) {
            drawPixel(i, y - y1, color); // Top span
            drawPixel(i, y + y1, color); // Bottom span
        }
        for (int i = x - y1; i <= x + y1; i++) {
            drawPixel(i, y - x1, color); // Left span
            drawPixel(i, y + x1, color); // Right span
        }

        if (dp <= 0) {
            dp += (4 * x1) + 6;
        } else {
            dp += 4 * (x1 - y1) + 10;
            y1--;
        }
        x1++;
    }
}

void SSD1306::draw_64(uint64_t* bitmap) {
    std::memcpy(this->frameBuffer, bitmap, 1024);
}

void SSD1306::renderDisplay(int startPage, int endPage) {

    uint8_t pageBuffer[129];   // Page buffer with control byte for data
    pageBuffer[0] = 0x40;

    for(int page = 0; page < 8; page++) {
        for(int col = 0; col < 128; col++) {

            pageBuffer[col + 1] = ((this->frameBuffer[col]) >> (page*8)) & 0xFF;
        }
        
        if(setCursor(0, page) < 3) {
            std::cout << "Error setting cursor" << std::endl;
        } 
        else {
            if(write(this->file, pageBuffer, sizeof(pageBuffer)) != sizeof(pageBuffer)) {
                std::cout << "There was an error writing page" << std::endl;
                break;
            }
        }
    }
}

void SSD1306::startHorizontalScroll(int startPage, int endPage, int direction, int speed) {

    if((startPage < 0 || startPage > 7) || (endPage < 0 || endPage > 7)) {
        std::cout << "Horizontal scrolling page index error : [" << startPage << " : " << endPage << "]" << std::endl;
        return;
    }
    if(direction != 1 && direction != 0) {
        std::cout << "Horizontal scrolling direction error : " << direction << std::endl;
        return;
    }
    if(speed < 0 ||speed > 7) {
        std::cout << "Horizontal scrolling speed error : " << speed << std::endl;
        return;
    }

    uint8_t scrollDir = (direction == 1) ? 0x26 : 0x27;

    const uint8_t commands[] = {
        scrollDir,
        0x00,
        static_cast<uint8_t>(startPage),
        static_cast<uint8_t>(speed),
        static_cast<uint8_t>(endPage),
        0x00,
        0xFF
    };

    int scrollStatus = sendCommand(commands, sizeof(commands));
    if(scrollStatus != (sizeof(commands) + 1)) {
        std::cerr << "Scrolling configuration failed: (" << scrollStatus << ") " << strerror(scrollStatus) << " while writing" << std::endl;
        return;
    }
    const uint8_t start_scroll = 0x2F;
    scrollStatus = sendCommand(&start_scroll, 1);
    if(scrollStatus != 2) {
        std::cerr << "Starting scrolling failed: (" << scrollStatus << ") " << strerror(scrollStatus) << " while writing" << std::endl;
    }
}

void SSD1306::startDiagonalRightScrl(int startPage, int endPage) {

    uint8_t start = (uint8_t)startPage;
    uint8_t end = (uint8_t)endPage;

    const uint8_t commands[] = {
        0x2A,
        0x00,
        start,
        0x00,
        end,
        0x3F
    };

    sendCommand(commands, sizeof(commands));
    const uint8_t start_scroll = 0x2F;
    sendCommand(&start_scroll, 1);
}

void SSD1306::startDiagonalLeftScrl(int startPage, int endPage) {

    uint8_t start = (uint8_t)startPage;
    uint8_t end = (uint8_t)endPage;
    const uint8_t start_scroll = 0x2F;


    // Set  vertical scrolling area
    const uint8_t commands[] = {
        0xA3,
        0x00,
        0x40
    };
    
    if(sendCommand(commands, sizeof(commands)) == sizeof(commands) + 1) {

        const uint8_t diagCom[] = {
            0x2A,
            0x00,
            start,
            0x00,
            end,
            0x3F
        };
        
        sendCommand(diagCom, sizeof(diagCom));
    }

    std::cout << "Started diag scroll " << (sendCommand(&start_scroll, 1) == 2) << std::endl;;
}

void SSD1306::stopScroll() {
    const uint8_t stop_scroll = 0x2E;
    sendCommand(&stop_scroll, 1);
}

uint8_t* SSD1306::ASCIImap(char c) {

    static unsigned char font_bitmap_6x8[] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x38,0x74,0x5c,0x74,0x38,0x00,0x00,0x00,
        0x38,0x74,0x7c,0x74,0x38,0x00,0x00,0x00,
        0x18,0x3c,0x78,0x3c,0x18,0x00,0x00,0x00,
        0x10,0x38,0x7c,0x38,0x10,0x00,0x00,0x00,
        0x18,0x14,0x7c,0x14,0x18,0x00,0x00,0x00,
        0x30,0x18,0x7c,0x18,0x30,0x00,0x00,0x00,
        0x00,0x10,0x38,0x10,0x00,0x00,0x00,0x00,
        0xfe,0xee,0xc6,0xee,0xfe,0x00,0x00,0x00,
        0x00,0x10,0x28,0x10,0x00,0x00,0x00,0x00,
        0xfe,0xee,0xc6,0xee,0xfe,0x00,0x00,0x00,
        0x20,0x50,0x34,0x0c,0x1c,0x00,0x00,0x00,
        0x00,0x28,0x74,0x28,0x00,0x00,0x00,0x00,
        0x60,0x38,0x04,0x08,0x00,0x00,0x00,0x00,
        0x60,0x38,0x04,0x34,0x1c,0x00,0x00,0x00,
        0x00,0x10,0x28,0x10,0x00,0x00,0x00,0x00,
        0x00,0x7c,0x38,0x10,0x00,0x00,0x00,0x00,
        0x00,0x10,0x38,0x7c,0x00,0x00,0x00,0x00,
        0x00,0x28,0x7c,0x28,0x00,0x00,0x00,0x00,
        0x00,0x5c,0x00,0x5c,0x00,0x00,0x00,0x00,
        0x18,0xfc,0x04,0xfc,0x04,0x00,0x00,0x00,
        0x90,0xa8,0x48,0x54,0x24,0x00,0x00,0x00,
        0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00,
        0x00,0xa8,0xfc,0xa8,0x00,0x00,0x00,0x00,
        0x00,0x08,0x7c,0x08,0x00,0x00,0x00,0x00,
        0x00,0x20,0x7c,0x20,0x00,0x00,0x00,0x00,
        0x10,0x10,0x10,0x38,0x10,0x00,0x00,0x00,
        0x10,0x38,0x10,0x10,0x10,0x00,0x00,0x00,
        0x30,0x20,0x20,0x20,0x20,0x00,0x00,0x00,
        0x10,0x38,0x10,0x38,0x10,0x00,0x00,0x00,
        0x40,0x60,0x70,0x60,0x40,0x00,0x00,0x00,
        0x10,0x30,0x70,0x30,0x10,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x5c,0x00,0x00,0x00,0x00,0x00,
        0x00,0x0c,0x00,0x0c,0x00,0x00,0x00,0x00,
        0x28,0x7c,0x28,0x7c,0x28,0x00,0x00,0x00,
        0x00,0x50,0xec,0x28,0x00,0x00,0x00,0x00,
        0x44,0x2a,0x34,0x58,0x24,0x00,0x00,0x00,
        0x20,0x58,0x54,0x24,0x50,0x00,0x00,0x00,
        0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,
        0x00,0x38,0x44,0x00,0x00,0x00,0x00,0x00,
        0x00,0x44,0x38,0x00,0x00,0x00,0x00,0x00,
        0x00,0x54,0x38,0x54,0x00,0x00,0x00,0x00,
        0x00,0x10,0x38,0x10,0x00,0x00,0x00,0x00,
        0x00,0x80,0x40,0x00,0x00,0x00,0x00,0x00,
        0x08,0x08,0x08,0x08,0x00,0x00,0x00,0x00,
        0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,
        0x00,0x60,0x18,0x04,0x00,0x00,0x00,0x00,
        0x38,0x44,0x44,0x38,0x00,0x00,0x00,0x00,
        0x00,0x08,0x7c,0x00,0x00,0x00,0x00,0x00,
        0x48,0x64,0x54,0x48,0x00,0x00,0x00,0x00,
        0x44,0x54,0x54,0x28,0x00,0x00,0x00,0x00,
        0x20,0x30,0x28,0x7c,0x00,0x00,0x00,0x00,
        0x5c,0x54,0x54,0x24,0x00,0x00,0x00,0x00,
        0x38,0x54,0x54,0x20,0x00,0x00,0x00,0x00,
        0x04,0x64,0x14,0x0c,0x00,0x00,0x00,0x00,
        0x28,0x54,0x54,0x28,0x00,0x00,0x00,0x00,
        0x08,0x54,0x54,0x38,0x00,0x00,0x00,0x00,
        0x00,0x00,0x50,0x00,0x00,0x00,0x00,0x00,
        0x00,0x80,0x50,0x00,0x00,0x00,0x00,0x00,
        0x00,0x10,0x28,0x44,0x00,0x00,0x00,0x00,
        0x00,0x28,0x28,0x28,0x00,0x00,0x00,0x00,
        0x00,0x44,0x28,0x10,0x00,0x00,0x00,0x00,
        0x00,0x54,0x14,0x08,0x00,0x00,0x00,0x00,
        0x38,0x44,0x54,0x54,0x08,0x00,0x00,0x00,
        0x78,0x14,0x14,0x78,0x00,0x00,0x00,0x00,
        0x7c,0x54,0x54,0x28,0x00,0x00,0x00,0x00,
        0x38,0x44,0x44,0x44,0x00,0x00,0x00,0x00,
        0x7c,0x44,0x44,0x38,0x00,0x00,0x00,0x00,
        0x7c,0x54,0x54,0x44,0x00,0x00,0x00,0x00,
        0x7c,0x14,0x14,0x04,0x00,0x00,0x00,0x00,
        0x38,0x44,0x44,0x68,0x00,0x00,0x00,0x00,
        0x7c,0x10,0x10,0x7c,0x00,0x00,0x00,0x00,
        0x00,0x44,0x7c,0x44,0x00,0x00,0x00,0x00,
        0x30,0x40,0x40,0x3c,0x00,0x00,0x00,0x00,
        0x7c,0x10,0x28,0x44,0x00,0x00,0x00,0x00,
        0x7c,0x40,0x40,0x40,0x00,0x00,0x00,0x00,
        0x7c,0x10,0x10,0x7c,0x00,0x00,0x00,0x00,
        0x7c,0x08,0x10,0x7c,0x00,0x00,0x00,0x00,
        0x38,0x44,0x44,0x38,0x00,0x00,0x00,0x00,
        0x7c,0x14,0x14,0x08,0x00,0x00,0x00,0x00,
        0x38,0x44,0x44,0xb8,0x00,0x00,0x00,0x00,
        0x7c,0x14,0x14,0x68,0x00,0x00,0x00,0x00,
        0x48,0x54,0x54,0x24,0x00,0x00,0x00,0x00,
        0x04,0x04,0x7c,0x04,0x04,0x00,0x00,0x00,
        0x3c,0x40,0x40,0x3c,0x00,0x00,0x00,0x00,
        0x1c,0x60,0x60,0x1c,0x00,0x00,0x00,0x00,
        0x1c,0x60,0x18,0x60,0x1c,0x00,0x00,0x00,
        0x4c,0x30,0x10,0x6c,0x00,0x00,0x00,0x00,
        0x00,0x1c,0x60,0x1c,0x00,0x00,0x00,0x00,
        0x64,0x54,0x4c,0x44,0x00,0x00,0x00,0x00,
        0x00,0x7c,0x44,0x00,0x00,0x00,0x00,0x00,
        0x00,0x0c,0x30,0x40,0x00,0x00,0x00,0x00,
        0x00,0x44,0x7c,0x00,0x00,0x00,0x00,0x00,
        0x00,0x08,0x04,0x08,0x00,0x00,0x00,0x00,
        0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,
        0x00,0x04,0x08,0x00,0x00,0x00,0x00,0x00,
        0x00,0x68,0x28,0x70,0x00,0x00,0x00,0x00,
        0x7e,0x48,0x48,0x30,0x00,0x00,0x00,0x00,
        0x00,0x30,0x48,0x48,0x00,0x00,0x00,0x00,
        0x30,0x48,0x48,0x7c,0x00,0x00,0x00,0x00,
        0x30,0x58,0x58,0x50,0x00,0x00,0x00,0x00,
        0x10,0x78,0x14,0x04,0x00,0x00,0x00,0x00,
        0x10,0xa8,0xa8,0x78,0x00,0x00,0x00,0x00,
        0x7c,0x08,0x08,0x70,0x00,0x00,0x00,0x00,
        0x00,0x48,0x7a,0x40,0x00,0x00,0x00,0x00,
        0x00,0x80,0x80,0x7a,0x00,0x00,0x00,0x00,
        0x7c,0x10,0x28,0x40,0x00,0x00,0x00,0x00,
        0x00,0x42,0x7e,0x40,0x00,0x00,0x00,0x00,
        0x78,0x10,0x10,0x78,0x00,0x00,0x00,0x00,
        0x78,0x08,0x08,0x70,0x00,0x00,0x00,0x00,
        0x30,0x48,0x48,0x30,0x00,0x00,0x00,0x00,
        0xf8,0x48,0x48,0x30,0x00,0x00,0x00,0x00,
        0x30,0x48,0x48,0xf8,0x00,0x00,0x00,0x00,
        0x00,0x78,0x10,0x08,0x00,0x00,0x00,0x00,
        0x50,0x58,0x68,0x28,0x00,0x00,0x00,0x00,
        0x08,0x3c,0x48,0x48,0x00,0x00,0x00,0x00,
        0x38,0x40,0x40,0x78,0x00,0x00,0x00,0x00,
        0x18,0x60,0x60,0x18,0x00,0x00,0x00,0x00,
        0x78,0x20,0x20,0x78,0x00,0x00,0x00,0x00,
        0x48,0x30,0x30,0x48,0x00,0x00,0x00,0x00,
        0x18,0xa0,0xa0,0x78,0x00,0x00,0x00,0x00,
        0x48,0x68,0x58,0x48,0x00,0x00,0x00,0x00,
        0x00,0x18,0x24,0x42,0x00,0x00,0x00,0x00,
        0x00,0x00,0x7e,0x00,0x00,0x00,0x00,0x00,
        0x00,0x42,0x24,0x18,0x00,0x00,0x00,0x00,
        0x10,0x08,0x10,0x08,0x00,0x00,0x00,0x00,
        0x60,0x50,0x48,0x50,0x60,0x00,0x00,0x00,
        0x38,0x44,0xc4,0x44,0x00,0x00,0x00,0x00,
        0x38,0x42,0x40,0x7a,0x00,0x00,0x00,0x00,
        0x30,0x58,0x5a,0x51,0x00,0x00,0x00,0x00,
        0x28,0x4a,0x31,0x42,0x00,0x00,0x00,0x00,
        0x48,0x2a,0x70,0x42,0x00,0x00,0x00,0x00,
        0x48,0x29,0x72,0x40,0x00,0x00,0x00,0x00,
        0x48,0x28,0x72,0x40,0x00,0x00,0x00,0x00,
        0x00,0x30,0xc8,0x48,0x00,0x00,0x00,0x00,
        0x30,0x5a,0x59,0x52,0x00,0x00,0x00,0x00,
        0x30,0x5a,0x58,0x52,0x00,0x00,0x00,0x00,
        0x30,0x59,0x5a,0x50,0x00,0x00,0x00,0x00,
        0x00,0x4a,0x78,0x42,0x00,0x00,0x00,0x00,
        0x00,0x4a,0x79,0x42,0x00,0x00,0x00,0x00,
        0x00,0x49,0x7a,0x40,0x00,0x00,0x00,0x00,
        0x79,0x14,0x15,0x78,0x00,0x00,0x00,0x00,
        0x78,0x14,0x15,0x78,0x00,0x00,0x00,0x00,
        0x7c,0x54,0x56,0x45,0x00,0x00,0x00,0x00,
        0x68,0x38,0x70,0x58,0x58,0x00,0x00,0x00,
        0x78,0x14,0x7c,0x54,0x00,0x00,0x00,0x00,
        0x30,0x4a,0x49,0x32,0x00,0x00,0x00,0x00,
        0x30,0x4a,0x48,0x32,0x00,0x00,0x00,0x00,
        0x30,0x49,0x4a,0x30,0x00,0x00,0x00,0x00,
        0x38,0x42,0x41,0x7a,0x00,0x00,0x00,0x00,
        0x38,0x41,0x42,0x78,0x00,0x00,0x00,0x00,
        0x18,0xa2,0xa0,0x7a,0x00,0x00,0x00,0x00,
        0x30,0x4a,0x48,0x32,0x00,0x00,0x00,0x00,
        0x3c,0x41,0x40,0x3d,0x00,0x00,0x00,0x00,
        0x30,0x48,0xcc,0x48,0x00,0x00,0x00,0x00,
        0x50,0x7c,0x52,0x46,0x00,0x00,0x00,0x00,
        0x02,0x2e,0x70,0x2e,0x02,0x00,0x00,0x00,
        0x7e,0x12,0x1c,0x38,0x50,0x00,0x00,0x00,
        0x90,0x7c,0x12,0x12,0x00,0x00,0x00,0x00,
        0x48,0x2a,0x71,0x40,0x00,0x00,0x00,0x00,
        0x00,0x48,0x7a,0x41,0x00,0x00,0x00,0x00,
        0x30,0x48,0x4a,0x31,0x00,0x00,0x00,0x00,
        0x38,0x40,0x42,0x79,0x00,0x00,0x00,0x00,
        0x7a,0x09,0x0a,0x71,0x00,0x00,0x00,0x00,
        0x7e,0x19,0x22,0x7d,0x00,0x00,0x00,0x00,
        0x00,0x24,0x2a,0x2c,0x00,0x00,0x00,0x00,
        0x00,0x24,0x2a,0x24,0x00,0x00,0x00,0x00,
        0x20,0x50,0x4a,0x20,0x00,0x00,0x00,0x00,
        0x60,0x20,0x20,0x20,0x20,0x00,0x00,0x00,
        0x20,0x20,0x20,0x20,0x60,0x00,0x00,0x00,
        0x2e,0x10,0x48,0x54,0x70,0x00,0x00,0x00,
        0x2e,0x10,0x48,0x64,0xf2,0x00,0x00,0x00,
        0x00,0x20,0x7a,0x20,0x00,0x00,0x00,0x00,
        0x20,0x50,0x20,0x50,0x00,0x00,0x00,0x00,
        0x50,0x20,0x50,0x20,0x00,0x00,0x00,0x00,
        0x55,0xaa,0x55,0xaa,0x55,0x00,0x00,0x00,
        0x55,0xbb,0x55,0xee,0x55,0x00,0x00,0x00,
        0x55,0xff,0xaa,0xff,0x55,0x00,0x00,0x00,
        0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,
        0x08,0x08,0xff,0x00,0x00,0x00,0x00,0x00,
        0x14,0x14,0xff,0x00,0x00,0x00,0x00,0x00,
        0x08,0xff,0x00,0xff,0x00,0x00,0x00,0x00,
        0x08,0xf8,0x08,0xf8,0x00,0x00,0x00,0x00,
        0x14,0x14,0xfc,0x00,0x00,0x00,0x00,0x00,
        0x14,0xf7,0x00,0xff,0x00,0x00,0x00,0x00,
        0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,
        0x14,0xf4,0x04,0xfc,0x00,0x00,0x00,0x00,
        0x14,0x17,0x10,0x1f,0x00,0x00,0x00,0x00,
        0x08,0x0f,0x08,0x0f,0x00,0x00,0x00,0x00,
        0x14,0x14,0x1f,0x00,0x00,0x00,0x00,0x00,
        0x08,0x08,0xf8,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x0f,0x08,0x08,0x00,0x00,0x00,
        0x08,0x08,0x0f,0x08,0x08,0x00,0x00,0x00,
        0x08,0x08,0xf8,0x08,0x08,0x00,0x00,0x00,
        0x00,0x00,0xff,0x08,0x08,0x00,0x00,0x00,
        0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x00,
        0x08,0x08,0xff,0x08,0x08,0x00,0x00,0x00,
        0x00,0x00,0xff,0x14,0x14,0x00,0x00,0x00,
        0x00,0xff,0x00,0xff,0x08,0x00,0x00,0x00,
        0x00,0x1f,0x10,0x17,0x14,0x00,0x00,0x00,
        0x00,0xfc,0x04,0xf4,0x14,0x00,0x00,0x00,
        0x14,0x17,0x10,0x17,0x14,0x00,0x00,0x00,
        0x14,0xf4,0x04,0xf4,0x14,0x00,0x00,0x00,
        0x00,0xff,0x00,0xf7,0x14,0x00,0x00,0x00,
        0x14,0x14,0x14,0x14,0x14,0x00,0x00,0x00,
        0x14,0xf7,0x00,0xf7,0x14,0x00,0x00,0x00,
        0x14,0x14,0x17,0x14,0x14,0x00,0x00,0x00,
        0x08,0x0f,0x08,0x0f,0x08,0x00,0x00,0x00,
        0x14,0x14,0xf4,0x14,0x14,0x00,0x00,0x00,
        0x08,0xf8,0x08,0xf8,0x08,0x00,0x00,0x00,
        0x00,0x0f,0x08,0x0f,0x08,0x00,0x00,0x00,
        0x00,0x00,0x1f,0x14,0x14,0x00,0x00,0x00,
        0x00,0x00,0xfc,0x14,0x14,0x00,0x00,0x00,
        0x00,0xf8,0x08,0xf8,0x08,0x00,0x00,0x00,
        0x08,0xff,0x08,0xff,0x08,0x00,0x00,0x00,
        0x14,0x14,0xff,0x14,0x14,0x00,0x00,0x00,
        0x08,0x08,0x0f,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0xf8,0x08,0x08,0x00,0x00,0x00,
        0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,
        0xf0,0xf0,0xf0,0xf0,0xf0,0x00,0x00,0x00,
        0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,
        0x0f,0x0f,0x0f,0x0f,0x0f,0x00,0x00,0x00,
        0x30,0x48,0x48,0x30,0x48,0x00,0x00,0x00,
        0xfc,0x4a,0x4a,0x3c,0x00,0x00,0x00,0x00,
        0x00,0x7e,0x02,0x02,0x00,0x00,0x00,0x00,
        0x00,0x7c,0x04,0x7c,0x00,0x00,0x00,0x00,
        0x62,0x56,0x4a,0x42,0x66,0x00,0x00,0x00,
        0x38,0x44,0x44,0x3c,0x04,0x00,0x00,0x00,
        0xf8,0x40,0x40,0x38,0x40,0x00,0x00,0x00,
        0x02,0x04,0x78,0x06,0x02,0x00,0x00,0x00,
        0x10,0x28,0xee,0x28,0x10,0x00,0x00,0x00,
        0x38,0x54,0x54,0x54,0x38,0x00,0x00,0x00,
        0x58,0x64,0x04,0x64,0x58,0x00,0x00,0x00,
        0x32,0x4d,0x49,0x30,0x00,0x00,0x00,0x00,
        0x30,0x48,0x78,0x48,0x30,0x00,0x00,0x00,
        0x50,0x28,0x58,0x48,0x34,0x00,0x00,0x00,
        0x00,0x3c,0x4a,0x4a,0x00,0x00,0x00,0x00,
        0x7c,0x02,0x02,0x7c,0x00,0x00,0x00,0x00,
        0x54,0x54,0x54,0x54,0x00,0x00,0x00,0x00,
        0x48,0x48,0x5c,0x48,0x48,0x00,0x00,0x00,
        0x40,0x62,0x54,0x48,0x00,0x00,0x00,0x00,
        0x00,0x48,0x54,0x62,0x00,0x00,0x00,0x00,
        0x00,0x00,0xf8,0x04,0x0c,0x00,0x00,0x00,
        0x30,0x20,0x1f,0x00,0x00,0x00,0x00,0x00,
        0x10,0x54,0x54,0x10,0x00,0x00,0x00,0x00,
        0x48,0x24,0x48,0x24,0x00,0x00,0x00,0x00,
        0x00,0x08,0x14,0x08,0x00,0x00,0x00,0x00,
        0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
        0x20,0x40,0x30,0x0c,0x04,0x00,0x00,0x00,
        0x00,0x0e,0x02,0x0c,0x00,0x00,0x00,0x00,
        0x00,0x12,0x1a,0x14,0x00,0x00,0x00,0x00,
        0x00,0x38,0x38,0x38,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

    // Endpoints to restrict charcater set
    uint8_t char_set_start = 0x00;
    uint8_t char_set_end = 0xFF;

    if(char_set_start < 0 || char_set_end > 255) {
        return nullptr;
    }
    else {
        int char_index = (c - char_set_start) * 8;
        return &font_bitmap_6x8[char_index];
    }
}
