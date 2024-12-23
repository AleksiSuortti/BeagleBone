#ifndef BBB_GPIO_H
#define BBB_GPIO_H

namespace BBB_gpio {

    void pinMode(int pin, const std::string& mode);

    float analogRead(int analog_channel);

    int digitalRead(int digital_pin);

    void digitalWrite(int digital_pin, int value);
}


#endif // BBB_GPIO_H