#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "xtensa/core-macros.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/io_mux_reg.h"
#include "sdkconfig.h"
#include "flipdot.h"
#include "fill.h"
#include "main.h"
#include "scroll.h"
#include "snake.h"
#include "firework.h"
#include "wifi_manager.h"
#include "web_server.h"

static const char *TAG = "Main";

static const char *MODE_NAMES[] = {
    [MODE_SCROLL]   = "SCROLL",
    [MODE_SNAKE]    = "SNAKE",
    [MODE_FIREWORK] = "FIREWORK",
    [MODE_EMPTY]    = "EMPTY",
};

// Volatile: 32-bit aligned writes on Xtensa are atomic, sufficient for infrequent mode changes.
static volatile sys_mode_t mode = MODE_SCROLL;

void mode_set(sys_mode_t new_mode) { mode = new_mode; }
sys_mode_t mode_get(void)          { return mode; }

const char *mode_name(sys_mode_t m)
{
    if (m >= 0 && m < MODE_MAX) return MODE_NAMES[m];
    return "UNKNOWN";
}

void app_main()
{
    ESP_LOGI(TAG, "Flipdot Display Controller System startup");

    gpio_set_direction(PIN_BUTTON, GPIO_MODE_INPUT);

    flipdot_init();
    snake_init();
    firework_init();

    wifi_manager_init();
    web_server_start();

    dotboard_t clean_board;
    fill_on_off(&clean_board, true);

    bool button_held = false;
    sys_mode_t prev_mode = mode;

    while (1)
    {
        // Physical button cycles mode on release
        if (!gpio_get_level(PIN_BUTTON))
        {
            button_held = true;
        }
        else if (button_held)
        {
            mode_set(mode == MODE_MAX - 1 ? 0 : mode + 1);
            button_held = false;
        }

        sys_mode_t current = mode_get();

        // Clear board whenever mode changes (button or web)
        if (current != prev_mode)
        {
            write_dotboard(&clean_board, false);
            ESP_LOGI(TAG, "Mode changed to %s", mode_name(current));
            prev_mode = current;
        }

        switch (current)
        {
        case MODE_SCROLL:
            scroll_update();
            break;

        case MODE_SNAKE:
            snake_update();
            break;

        case MODE_FIREWORK:
        {
            static uint16_t fw_trigger_counter = 0;
            if (fw_trigger_counter == 0)
            {
                trigger_firework(1 + rand() % 10);
            }
            fw_trigger_counter++;
            if (fw_trigger_counter >= 200)
            {
                fw_trigger_counter = 0;
            }
            firework_update();
            break;
        }

        case MODE_EMPTY:
            break;

        default:
            break;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
