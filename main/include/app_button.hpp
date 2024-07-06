#pragma once

#include <vector>
#include "__base__.hpp"

typedef enum 
{
    BUTTON_IDLE = 0,
    BUTTON_MENU,
    BUTTON_PLAY,
    BUTTON_UP,
    BUTTON_DOWN
} button_name_t;

typedef struct 
{
    button_name_t key;  /**< button index */
    gpio_num_t pin;     /**< pin on hardware */
} key_config_t;

class AppButton : public Subject
{
public:
    std::vector<key_config_t> key_configs;
    button_name_t pressed;

    uint8_t menu;

    AppButton();
    ~AppButton();

    void run();
};
