#include "app_camera.hpp"
#include "esp_log.h"
#include "esp_system.h"

const static char TAG[] = "App/Camera";

AppCamera::AppCamera(const pixformat_t pixel_fromat,
                     const framesize_t frame_size,
                     const uint8_t fb_count,
                     QueueHandle_t queue_o) : Frame(nullptr, queue_o, nullptr)
{
    ESP_LOGI(TAG, "Camera device is %s", CAMERA_MODULE_NAME);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = CAMERA_PIN_D0;
    config.pin_d1       = CAMERA_PIN_D1;
    config.pin_d2       = CAMERA_PIN_D2;
    config.pin_d3       = CAMERA_PIN_D3;
    config.pin_d4       = CAMERA_PIN_D4;
    config.pin_d5       = CAMERA_PIN_D5;
    config.pin_d6       = CAMERA_PIN_D6;
    config.pin_d7       = CAMERA_PIN_D7;
    config.pin_xclk     = CAMERA_PIN_XCLK;
    config.pin_pclk     = CAMERA_PIN_PCLK;
    config.pin_vsync    = CAMERA_PIN_VSYNC;
    config.pin_href     = CAMERA_PIN_HREF;
    config.pin_sscb_sda = CAMERA_PIN_SIOD;
    config.pin_sscb_scl = CAMERA_PIN_SIOC;
    config.pin_pwdn     = CAMERA_PIN_PWDN;
    config.pin_reset    = CAMERA_PIN_RESET;
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.pixel_format = pixel_fromat;
    config.frame_size   = frame_size;
    config.jpeg_quality = 12;
    config.fb_count     = fb_count;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); // flip it back
    s->set_sharpness(s, 2);
    s->set_awb_gain(s, 2);
}

static void task(AppCamera *self)
{
    ESP_LOGD(TAG, "Start");
    while (true) 
    {
        if (self->queue_o == nullptr)
            break;

        camera_fb_t *frame = esp_camera_fb_get();
        if (frame)
            xQueueSend(self->queue_o, &frame, portMAX_DELAY);
    }
    ESP_LOGD(TAG, "Stop");
    vTaskDelete(NULL);
}

void AppCamera::run()
{
    xTaskCreatePinnedToCore((TaskFunction_t)task, TAG, 2 * 1024, this, 5, NULL, 0);
}
