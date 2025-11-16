/*****************************************************************************
 * | File        :   rgb_lcd_port.h
 * | Author      :   Waveshare team
 * | Function    :   Hardware underlying interface
 * | Info        :
 *                   This header file contains configuration and function 
 *                   declarations for the RGB LCD driver interface.
 *----------------
 * | Version     :   V1.0
 * | Date        :   2024-11-19
 * | Info        :   Basic version
 *
 ******************************************************************************/

#ifndef _RGB_LCD_H_
#define _RGB_LCD_H_

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "../io_extension/io_extension.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"

/* LCD Resolution and Timing */
#define EXAMPLE_LCD_H_RES               (1024)
#define EXAMPLE_LCD_V_RES               (600)
/* PCLK: tu as constaté que 25 MHz est le mieux stable → on garde 25 MHz */
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (25 * 1000 * 1000)

/* Couleur / interface */
#define EXAMPLE_LCD_BIT_PER_PIXEL       (16)
#define EXAMPLE_RGB_BIT_PER_PIXEL       (16)
#define EXAMPLE_RGB_DATA_WIDTH          (16)

/* MOD: triple buffering pour lisser le rendu */
#define EXAMPLE_LCD_RGB_BUFFER_NUMS     (3)

#define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * 10)

/* GPIOs */
#define EXAMPLE_LCD_IO_RGB_DISP         (-1)
#define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_3)
#define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_46)
#define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_5)
#define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_7)

/* Data */
#define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_14)
#define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_38)
#define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_18)
#define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_17)
#define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_10)
#define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_39)
#define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_0)
#define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_45)
#define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_48)
#define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_47)
#define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_21)
#define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_1)
#define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_2)
#define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_42)
#define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_41)
#define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_40)

/* Reset / Backlight */
#define EXAMPLE_LCD_IO_RST              (-1)
#define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL)

/* API */
esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init();
void wavesahre_rgb_lcd_bl_on();
void wavesahre_rgb_lcd_bl_off();
void wavesahre_rgb_lcd_display_window(int16_t Xstart, int16_t Ystart, int16_t Xend, int16_t Yend, uint8_t *Image);
void wavesahre_rgb_lcd_display(uint8_t *Image);
void waveshare_get_frame_buffer(void **buf1, void **buf2);

#endif // _RGB_LCD_H_
/*****************************************************************************
 * rgb_lcd_port.h - Waveshare ESP32-S3 7" LCD RGB driver (adapté perf)
 ******************************************************************************/
/*****************************************************************************
 * rgb_lcd_port.h - Baseline Arduino (double buffer, direct mode)
 ******************************************************************************/
// #ifndef _RGB_LCD_H_
// #define _RGB_LCD_H_

// #include "esp_log.h"
// #include "esp_heap_caps.h"
// #include "../io_extension/io_extension.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_lcd_panel_ops.h"
// #include "esp_lcd_panel_rgb.h"

// /* Résolution */
// #define EXAMPLE_LCD_H_RES               (1024)
// #define EXAMPLE_LCD_V_RES               (600)

// /* PCLK (stable) - tu peux revenir à 30 MHz si l’image est OK */
// #define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (25 * 1000 * 1000)

// /* Couleur / format */
// #define EXAMPLE_LCD_BIT_PER_PIXEL       (16)
// #define EXAMPLE_RGB_BIT_PER_PIXEL       (16)
// #define EXAMPLE_RGB_DATA_WIDTH          (16)

// /* Double buffering (mode direct) */
// #define EXAMPLE_LCD_RGB_BUFFER_NUMS     (2)

// /* Bounce buffer (utilisé par le driver pour transferts partiels si besoin) */
// #define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * 10)

// /* Signaux */
// #define EXAMPLE_LCD_IO_RGB_DISP         (-1)
// #define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_3)
// #define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_46)
// #define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_5)
// #define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_7)

// /* Data lines */
// #define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_14)
// #define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_38)
// #define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_18)
// #define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_17)
// #define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_10)
// #define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_39)
// #define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_0)
// #define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_45)
// #define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_48)
// #define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_47)
// #define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_21)
// #define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_1)
// #define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_2)
// #define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_42)
// #define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_41)
// #define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_40)

// /* Backlight */
// #define EXAMPLE_LCD_IO_RST              (-1)
// #define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)
// #define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)
// #define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL)

// esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init();
// void wavesahre_rgb_lcd_bl_on();
// void wavesahre_rgb_lcd_bl_off();
// void wavesahre_rgb_lcd_display_window(int16_t Xstart, int16_t Ystart,
//                                       int16_t Xend, int16_t Yend, uint8_t *Image);
// void wavesahre_rgb_lcd_display(uint8_t *Image);
// void waveshare_get_frame_buffer(void **buf1, void **buf2);

// #endif