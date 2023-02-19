// C
// FreeRTOS
// ESP32
// App
#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio.h"


#define PIN_JIG_CONTROLLED_MAC  16
#define PIN_BUTTON              4
#define PIN_LED                 5
#define PIN_STM_RESET           18
#define PIN_LED_POWER           2
#define GPIO_INPUT_PIN_SEL  ((1ULL<<PIN_BUTTON))


static const char *TAG = "GPIO";

void gpio_toggle_led(bool status)
{
    gpio_set_level(PIN_LED, status);
}
//QueueHandle_t gpio_evt_queue = NULL;


// bool led_status = false;
// static void IRAM_ATTR gpio_isr_handler(void *args)
// {
//     gpio_set_level(PIN_LED, led_status);
//     //uint32_t gpio_num = (uint32_t) arg;
//     xQueueSendFromISR(gpio_evt_queue, &led_status, NULL);
//     led_status = !led_status;


//     //ESP_LOGD(TAG, "Interrupt");


//     // int pin_number = (int)args;
//     // if (pin_number == PIN_BUTTON) {
//     //     // Do mqtt and led toggle
//     // }
// }

// static void gpio_task_example(void* arg)
// {
//     uint32_t io_num;
//     for(;;) {
//         if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
//             printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
//             mqtt_publish_status(gpio_get_level(io_num));

//         }
//     }
// }


void gpio_init(void)
{
    //gpio_evt_queue = xQueueCreate(10, sizeof(bool));
    // Production pin to read MAC address
    gpio_pad_select_gpio(PIN_JIG_CONTROLLED_MAC); // Controlled by jig, run ESP32 normally or print MAC
    //gpio_pad_select_gpio(18); // STM32 reset
    //gpio_pad_select_gpio(21); // STM32, run normally or do OTA
    //gpio_pad_select_gpio(PIN_BUTTON); // Button
    gpio_pad_select_gpio(PIN_LED); // LED
    gpio_pad_select_gpio(PIN_LED_POWER); // LED


    gpio_set_direction(PIN_JIG_CONTROLLED_MAC, GPIO_MODE_INPUT);
    //gpio_set_direction(18, GPIO_MODE_OUTPUT);
    //gpio_set_direction(21, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED_POWER, GPIO_MODE_OUTPUT);

    gpio_set_level(PIN_LED_POWER, true);

    // gpio_config_t io_conf = {};
    // //interrupt of rising edge
    // io_conf.intr_type = GPIO_INTR_POSEDGE;
    // //bit mask of the pins, use GPIO4/5 here
    // io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // //set as input mode
    // io_conf.mode = GPIO_MODE_INPUT;
    // //enable pull-up mode
    // io_conf.pull_up_en = 1;
    // gpio_config(&io_conf);
    // gpio_install_isr_service(0);
    // //hook isr handler for specific gpio pin
    // gpio_isr_handler_add(PIN_BUTTON, gpio_isr_handler, (void*) PIN_BUTTON);
}
