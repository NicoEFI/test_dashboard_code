// ok fonctionne
#include <Arduino.h>
#include "lvgl_port.h"           // LVGL porting functions for integration with ESP32
#include "ui.h"                  // UI générée par SquareLine (fichiers C)
#include "ui/slider.h"
#include "ui/battery.h"

void setup() {
    Serial.begin(115200);

    static esp_lcd_panel_handle_t panel_handle = NULL; // LCD panel handle
    static esp_lcd_touch_handle_t tp_handle = NULL;    // Touch panel handle

    // Initialize GT911 capacitive touch controller (initialises I2C and IO_EXTENSION)
    tp_handle = touch_gt911_init();

    // Initialize Waveshare ESP32-S3 RGB LCD panel
    panel_handle = waveshare_esp32_s3_rgb_lcd_init();

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();

    // Initialize LVGL port with LCD and touch handles
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    ESP_LOGI(TAG, "Display LVGL demos");

    // Lock LVGL access, create UI, then unlock
    if (lvgl_port_lock(-1)) {
        // Create SquareLine UI (ui_init loads the screen)
        ui_init();

        // Create the slider (and its label) and initialize PWM to startup state
        lvgl_slider();

        // Create the battery label UI (battery module)
        battery_init_ui();

        lvgl_port_unlock();
    }
}

void loop() {
    // Appel non bloquant : la gestion interne de battery_update() utilise millis()
    battery_update();

    // Pas de delay() bloquant : on laisse la loop tourner.
    // On peut éventuellement yield pour laisser la tâche RTOS s'exécuter :
    delay(0); // court yield, non bloquant
}