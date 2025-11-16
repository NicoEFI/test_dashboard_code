#ifndef _LVGL_PORT_H_
#define _LVGL_PORT_H_

#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_types.h"
#include "../lvgl/lvgl.h"
#include "../rgb_lcd_port/rgb_lcd_port.h"
#include "../touch/gt911.h"

/* LVGL tick */
#define LVGL_PORT_TICK_PERIOD_MS    (2)

/* LVGL task */
#define LVGL_PORT_TASK_MAX_DELAY_MS (500)
#define LVGL_PORT_TASK_MIN_DELAY_MS (10)
#define LVGL_PORT_TASK_STACK_SIZE   (6 * 1024)
#define LVGL_PORT_TASK_PRIORITY     (2)
#define LVGL_PORT_TASK_CORE         (1)

/* Avoid tearing: ON, mode 2 (triple buffer + full refresh) */
#define LVGL_PORT_AVOID_TEAR_ENABLE     (1)
#if LVGL_PORT_AVOID_TEAR_ENABLE
#define LVGL_PORT_AVOID_TEAR_MODE       (2)   /* 3 buffers + full-refresh */
#define EXAMPLE_LVGL_PORT_ROTATION_DEGREE  (0)

#if LVGL_PORT_AVOID_TEAR_MODE == 1
#define LVGL_PORT_LCD_RGB_BUFFER_NUMS   (2)
#define LVGL_PORT_FULL_REFRESH          (1)
#elif LVGL_PORT_AVOID_TEAR_MODE == 2
#define LVGL_PORT_LCD_RGB_BUFFER_NUMS   (3)
#define LVGL_PORT_FULL_REFRESH          (1)
#elif LVGL_PORT_AVOID_TEAR_MODE == 3
#define LVGL_PORT_LCD_RGB_BUFFER_NUMS   (2)
#define LVGL_PORT_DIRECT_MODE           (1)
#endif

#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0
#define EXAMPLE_LVGL_PORT_ROTATION_0    (1)
#endif
#else
#define LVGL_PORT_LCD_RGB_BUFFER_NUMS   (1)
#define LVGL_PORT_FULL_REFRESH          (0)
#define LVGL_PORT_DIRECT_MODE           (0)
#endif

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle);
bool lvgl_port_lock(int timeout_ms);
void lvgl_port_unlock(void);
bool lvgl_port_notify_rgb_vsync(void);

#endif /* _LVGL_PORT_H_ */

// #ifndef _LVGL_PORT_H_
// #define _LVGL_PORT_H_

// #include <stdint.h>
// #include "esp_err.h"
// #include "esp_lcd_types.h"
// #include "../lvgl/lvgl.h"
// #include "../rgb_lcd_port/rgb_lcd_port.h"
// #include "../touch/gt911.h"

// #define LVGL_PORT_TICK_PERIOD_MS    (2)
// #define LVGL_PORT_TASK_MAX_DELAY_MS (500)
// #define LVGL_PORT_TASK_MIN_DELAY_MS (10)
// #define LVGL_PORT_TASK_STACK_SIZE   (6 * 1024)
// #define LVGL_PORT_TASK_PRIORITY     (2)
// #define LVGL_PORT_TASK_CORE         (1)

// /* Mode anti-tearing direct: double buffer + direct_mode */
// #define LVGL_PORT_AVOID_TEAR_ENABLE  (1)
// #define LVGL_PORT_AVOID_TEAR_MODE    (3)
// #define EXAMPLE_LVGL_PORT_ROTATION_DEGREE (0)

// #if LVGL_PORT_AVOID_TEAR_ENABLE
// #if LVGL_PORT_AVOID_TEAR_MODE == 3
// #define LVGL_PORT_LCD_RGB_BUFFER_NUMS (2)
// #define LVGL_PORT_DIRECT_MODE         (1)
// #else
// #error Garder mode 3 pour cette baseline
// #endif
// #else
// #define LVGL_PORT_LCD_RGB_BUFFER_NUMS (1)
// #endif

// esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle);
// bool lvgl_port_lock(int timeout_ms);
// void lvgl_port_unlock(void);
// bool lvgl_port_notify_rgb_vsync(void);

// #endif