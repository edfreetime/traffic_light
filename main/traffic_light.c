#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "portmacro.h"


// PIN Mapping
#define LED_RED GPIO_NUM_25
#define LED_YELLOW GPIO_NUM_26
#define LED_GREEN GPIO_NUM_27

// TL States
typedef enum {
    TL_RED, TL_YELLOW, TL_GREEN
} tl_state_t;

// Queue
QueueHandle_t tl_queue;

// Light Time
const int TL_RED_TIME = 100000; // 1 minutes
const int TL_GREEN_TIME = 100000; // 1 minutes
const int TL_YELLOW_TIME = 5000; // 5 seconds


// TL Task
void traffic_light_task(void *pvParameters) {
    tl_state_t state;

    while (1) {
        if (xQueueReceive(tl_queue, &state, portMAX_DELAY)) {
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
            }
        }
    }
}


// Controller Task
void controller_task(void *pvParameters) {
    tl_state_t state;

    while (1) {
        // Start with RED light for TL_RED_TIME
        state = TL_RED;
        xQueueSend(tl_queue, &state, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(TL_RED_TIME));

        // GREEN light for TL_GREEN_TIME
        state = TL_GREEN;
        xQueueSend(tl_queue, &state, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(TL_GREEN_TIME));

        // transition to RED over YELLOW for TL_YELLOW_TIME
        state = TL_YELLOW;
        xQueueSend(tl_queue, &state, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(TL_YELLOW_TIME));
    }
}


void app_main(void)
{
    // Configure GPIO Output
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 
            (1ULL << LED_RED) |
            (1ULL << LED_YELLOW) |
            (1ULL << LED_GREEN),
    };
    gpio_config(&io_conf);

    // Create queue
    tl_queue = xQueueCreate(1, sizeof(tl_state_t));

    // Create tasks
    xTaskCreate(traffic_light_task, "traffic_light", 2048, NULL, 5, NULL);
    xTaskCreate(controller_task, "controller", 2048, NULL, 5, NULL);
}
