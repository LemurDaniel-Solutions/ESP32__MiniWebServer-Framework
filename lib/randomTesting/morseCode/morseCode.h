#pragma once

#include <Arduino.h>
#include <string>
#include <vector>

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <./morseCode/mapping.h>

namespace morseCode
{
    class Morse
    {
    private:
        int _timeUnit = 500; // milliseconds
        gpio_num_t _pin;

        void _morseLetter(char letter)
        {
            std::vector<int> delays = codeToDelays(letter, _timeUnit);

            Serial.printf("%c | %s\n", letter, getMorseCode(letter).c_str());

            for (int delay : delays)
            {
                Serial.printf("  | Blinking %d ms\n", delay);
                gpio_set_level(_pin, 1);
                vTaskDelay(delay / portTICK_PERIOD_MS);
                gpio_set_level(_pin, 0);
                vTaskDelay(_timeUnit / portTICK_PERIOD_MS);
            }
        }

    public:
        Morse(gpio_num_t pin) : _pin(pin)
        {
            gpio_reset_pin(_pin);
            gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
        }

        void morse(const std::string message)
        {
            const int wordGap = 7 * _timeUnit;

            for (const char c : message)
            {
                if (c == ' ')
                    vTaskDelay(wordGap / portTICK_PERIOD_MS);
                else
                    _morseLetter(c);
            }
        }
    };
}
