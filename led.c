#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define BLINK_GPIO 8

static const char *TAG = "led";

static uint8_t s_led_state = 0;

static led_strip_handle_t led_strip;
static TaskHandle_t s_blink_task_handle = NULL;

static void blink(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 0, 10, 50);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

static void blink_task(void *pvParameter)
{
    /* Configure the peripheral according to the LED type */
    configure_led();

    while (true) {
        blink();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void stop_blink(void)
{
    if (s_blink_task_handle != NULL) {
        vTaskDelete(s_blink_task_handle);
        s_blink_task_handle = NULL;
    }
}

void led_blink(void)
{
    xTaskCreate(&blink_task, "blink_task", 2048, NULL, 5, &s_blink_task_handle);
}

void led_off() {
    stop_blink();
    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);
}

void on() {
    stop_blink();
    led_strip_set_pixel(led_strip, 0, 0, 10, 50);
    led_strip_refresh(led_strip);
}