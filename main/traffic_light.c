#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "esp_log.h"


// PIN Mapping
#define LED_RED GPIO_NUM_25
#define LED_YELLOW GPIO_NUM_26
#define LED_GREEN GPIO_NUM_27
#define BUTTON_PIN GPIO_NUM_33

// TL Modes
typedef enum {
    MODE_NORMAL = 0, 
    MODE_NIGHT,
    MODE_MAINTENANCE,
    MODE_COUNT
} tl_mode_t;

// Sound sensor event
typedef enum {
    EVENT_TOGGLE_MODE
} tl_event_t;

// TL States
typedef enum {
    TL_RED, TL_YELLOW, TL_GREEN, TL_YELLOW_BLINK, TL_ALL_BLINK
} tl_state_t;

// Queue
QueueHandle_t tl_queue;
QueueHandle_t event_queue;

// Light Time
const int TL_RED_TIME = 100000; // 1 minutes
const int TL_GREEN_TIME = 100000; // 1 minutes
const int TL_YELLOW_TIME = 5000; // 5 seconds
// const int TL_RED_TIME = 5000; // 5 seconds
// const int TL_GREEN_TIME = 5000; // 5 seconds
// const int TL_YELLOW_TIME = 1000; // 1 seconds


// TL Task
void traffic_light_task(void *pvParameters) {
    tl_state_t state;
    int yellow_on = 0;

    while (1) {
        if (xQueueReceive(tl_queue, &state, portMAX_DELAY)) {
            if(state == TL_YELLOW_BLINK) {
                while (1) {
                    yellow_on = !yellow_on;

                    gpio_set_level(LED_RED, 0);
                    gpio_set_level(LED_GREEN, 0);
                    gpio_set_level(LED_YELLOW, yellow_on);

                    if (xQueueReceive(tl_queue, &state, pdMS_TO_TICKS(500))) {
                        yellow_on = 0;
                        break;
                    }
                }
            }

            if (state == TL_ALL_BLINK) {
                int on = 0;

                while (1) {
                    on = !on;

                    gpio_set_level(LED_RED, on);
                    gpio_set_level(LED_GREEN, on);
                    gpio_set_level(LED_YELLOW, on);

                    if (xQueueReceive(tl_queue, &state, pdMS_TO_TICKS(500))) {
                        break;
                    }
                }
            }

            switch (state) {
                case TL_RED:
                    // Lights On RED
                    gpio_set_level(LED_RED, 1);
                    gpio_set_level(LED_YELLOW, 0);
                    gpio_set_level(LED_GREEN, 0);
                    break;
                case TL_YELLOW:
                    // Lights On YELLOW
                    gpio_set_level(LED_RED, 0);
                    gpio_set_level(LED_YELLOW, 1);
                    gpio_set_level(LED_GREEN, 0);
                    break;
                case TL_GREEN:
                    // Lights On GREEN
                    gpio_set_level(LED_RED, 0);
                    gpio_set_level(LED_YELLOW, 0);
                    gpio_set_level(LED_GREEN, 1);
                    break;
                default:
                    break;
            }
        }
    }
}

// Button Task
void button_task(void *pvParameters) {
    int last_state = 1;

    while (1) {
        int current = gpio_get_level(BUTTON_PIN);

        if (current == 0 && last_state == 1) {
            tl_event_t evt = EVENT_TOGGLE_MODE;
            xQueueSend(event_queue, &evt, portMAX_DELAY);
        }

        last_state = current;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


static void delay_with_event_check(int ms, tl_mode_t *mode)
{
    tl_event_t evt;
    int elapsed = 0;

    while (elapsed < ms) {
        if (xQueueReceive(event_queue, &evt, 0)) {
            if (evt == EVENT_TOGGLE_MODE) {
                *mode = (*mode == MODE_NORMAL) ? MODE_NIGHT : MODE_NORMAL;
                ESP_LOGI("CTRL", "Mode changed immediately");
                return; // keluar dari state sekarang
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
        elapsed += 50;
    }
}


// Controller Task
void controller_task(void *pvParameters) {
    tl_mode_t mode = MODE_NORMAL;
    tl_mode_t last_mode = MODE_NORMAL;
    tl_event_t evt;
    tl_state_t state;

    while (1) {
        // Check for non-pblocking event
        if (xQueueReceive(event_queue, &evt, 0)) {
            if (evt == EVENT_TOGGLE_MODE) {
                mode = (mode + 1) % MODE_COUNT;
                ESP_LOGI("CTRL", "Mode changed to %d", mode);
            }
        }

        // Transition mode
        if (mode != last_mode) {
            if (mode == MODE_NIGHT) {
                state = TL_YELLOW_BLINK;
                xQueueSend(tl_queue, &state, 0);
            }

            if (mode == MODE_MAINTENANCE) {
                state = TL_ALL_BLINK;
                xQueueSend(tl_queue, &state, 0);
            }

            last_mode = mode;
        }

        if (mode == MODE_NORMAL) {
            // Start with RED light for TL_RED_TIME
            state = TL_RED;
            xQueueSend(tl_queue, &state, portMAX_DELAY);
            delay_with_event_check(TL_RED_TIME, &mode);
            if (mode != MODE_NORMAL) continue;

            // GREEN light for TL_GREEN_TIME
            state = TL_GREEN;
            xQueueSend(tl_queue, &state, portMAX_DELAY);
            delay_with_event_check(TL_GREEN_TIME, &mode);
            if (mode != MODE_NORMAL) continue;

            // transition to RED over YELLOW for TL_YELLOW_TIME
            state = TL_YELLOW;
            xQueueSend(tl_queue, &state, portMAX_DELAY);
            delay_with_event_check(TL_YELLOW_TIME, &mode);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void app_main(void)
{
    // Configure LED GPIO Output
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 
            (1ULL << LED_RED) |
            (1ULL << LED_YELLOW) |
            (1ULL << LED_GREEN),
    };
    gpio_config(&io_conf);

    // Configure Button GPIO
    gpio_config_t button_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .pull_up_en = 1,
    };
    gpio_config(&button_conf);

    // Create queue
    tl_queue = xQueueCreate(1, sizeof(tl_state_t));
    event_queue = xQueueCreate(5, sizeof(tl_event_t));

    // Create tasks
    xTaskCreate(traffic_light_task, "traffic_light", 2048, NULL, 5, NULL);
    xTaskCreate(controller_task, "controller", 4096, NULL, 6, NULL);
    xTaskCreate(button_task, "button", 2048, NULL, 7, NULL);
}
