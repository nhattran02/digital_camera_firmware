#include "app_button.hpp"
#include <stdio.h>
#include <stdlib.h>
#include "esp_timer.h"
#include "esp_log.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PRESS_INTERVAL 500000

static const char *TAG = "App/Button";
static void initializeButtons(AppButton *self);

AppButton::AppButton() : 
    key_configs({{BUTTON_MENU, GPIO_NUM_2}, 
                {BUTTON_PLAY, GPIO_NUM_3}, 
                {BUTTON_UP, GPIO_NUM_48}, 
                {BUTTON_DOWN, GPIO_NUM_42}}),
                pressed(BUTTON_IDLE), menu(0)
{
    initializeButtons(this);
}

static void initializeButtons(AppButton *self)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    io_conf.pin_bit_mask = (1ULL << self->key_configs[0].pin) 
                         | (1ULL << self->key_configs[1].pin) 
                         | (1ULL << self->key_configs[2].pin) 
                         | (1ULL << self->key_configs[3].pin);
    gpio_config(&io_conf);
}


static void task(AppButton *self)
{
    int64_t backup_time = esp_timer_get_time();
    int64_t last_time = esp_timer_get_time();

    uint8_t menu_count = 0;

    while (true) 
    {
        backup_time = esp_timer_get_time();
        for (int i = 0; i < 4; i++) 
        {
            if (gpio_get_level(self->key_configs[i].pin) == 0) 
            {
                if (((backup_time - last_time) > PRESS_INTERVAL)) 
                {
                    self->pressed = self->key_configs[i].key;
                    ESP_LOGI(TAG, "Button[%d] is clicked", self->pressed);
                    if (self->pressed == BUTTON_MENU) 
                    {
                        self->menu++;
                        self->menu %= (MENU_MOTION_DETECTION + 1);
                    }

                    last_time = backup_time;
                    self->notify();

                    self->pressed = BUTTON_IDLE;
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void AppButton::run()
{
    xTaskCreatePinnedToCore((TaskFunction_t)task, TAG, 3 * 1024, this, 5, NULL, 0);
}