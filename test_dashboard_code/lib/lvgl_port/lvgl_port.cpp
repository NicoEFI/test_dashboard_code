#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "lvgl_port.h"

#define LVGL_PORT_H_RES (EXAMPLE_LCD_H_RES)
#define LVGL_PORT_V_RES (EXAMPLE_LCD_V_RES)

static const char *TAG = "lv_port";
static SemaphoreHandle_t lvgl_mux = NULL;
static TaskHandle_t lvgl_task_handle = NULL;

/* ===== Triple buffer full-refresh, rotation 0 ===== */
#if LVGL_PORT_FULL_REFRESH && (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0)
static void *lvgl_port_rgb_last_buf = NULL;
static void *lvgl_port_rgb_next_buf = NULL;
static void *lvgl_port_flush_next_buf = NULL;
#endif

/* Flush (non bloquant) pour triple buffer full-refresh */
static void flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

#if LVGL_PORT_FULL_REFRESH && (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0)
    /* Donner à l’IDF le buffer rendu par LVGL (color_map) tout de suite */
    drv->draw_buf->buf1 = color_map;
    drv->draw_buf->buf2 = lvgl_port_flush_next_buf;
    lvgl_port_flush_next_buf = color_map;

    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lvgl_port_rgb_next_buf = color_map;

    /* Pas d’attente VSYNC ici → fluidité */
#else
    /* Chemins non utilisés dans cette config */
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
#endif
    lv_disp_flush_ready(drv);
}

static lv_disp_t *display_init(esp_lcd_panel_handle_t panel_handle)
{
    assert(panel_handle);

    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;

    void *buf1 = NULL;
    void *buf2 = NULL;
    int buffer_size = LVGL_PORT_H_RES * LVGL_PORT_V_RES;

#if (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0) && LVGL_PORT_FULL_REFRESH
    /* Obtenir 3 framebuffers du driver */
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 3, &lvgl_port_rgb_last_buf, &buf1, &buf2));
    lvgl_port_rgb_next_buf = lvgl_port_rgb_last_buf;
    lvgl_port_flush_next_buf = buf2;
#else
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
#endif

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, buffer_size);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res   = LVGL_PORT_H_RES;
    disp_drv.ver_res   = LVGL_PORT_V_RES;
    disp_drv.flush_cb  = flush_callback;
    disp_drv.draw_buf  = &disp_buf;
    disp_drv.user_data = panel_handle;

#if LVGL_PORT_FULL_REFRESH
    disp_drv.full_refresh = 1;   /* Triple buffer + full refresh */
#endif

    return lv_disp_drv_register(&disp_drv);
}

/* Touch */
static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)indev_drv->user_data;
    if(!tp) { data->state = LV_INDEV_STATE_RELEASED; return; }

    uint16_t x,y; uint8_t cnt=0;
    esp_lcd_touch_read_data(tp);
    bool pressed = esp_lcd_touch_get_coordinates(tp, &x, &y, NULL, &cnt, 1);
    if (pressed && cnt>0) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static lv_indev_t *indev_init(esp_lcd_touch_handle_t tp)
{
    if(!tp) return NULL;
    static lv_indev_drv_t indev_drv_tp;
    lv_indev_drv_init(&indev_drv_tp);
    indev_drv_tp.type = LV_INDEV_TYPE_POINTER;
    indev_drv_tp.read_cb = touchpad_read;
    indev_drv_tp.user_data = tp;
    return lv_indev_drv_register(&indev_drv_tp);
}

/* Tick */
static void tick_increment(void *arg) { lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS); }
static esp_err_t tick_init(void)
{
    const esp_timer_create_args_t args = { .callback = &tick_increment, .name = "lv_tick" };
    esp_timer_handle_t h = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&args, &h));
    return esp_timer_start_periodic(h, LVGL_PORT_TICK_PERIOD_MS * 1000);
}

/* Task LVGL */
static void lvgl_port_task(void *arg)
{
    uint32_t delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
    for(;;) {
        if (lvgl_port_lock(-1)) {
            delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        if (delay_ms > LVGL_PORT_TASK_MAX_DELAY_MS) delay_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
        if (delay_ms < LVGL_PORT_TASK_MIN_DELAY_MS) delay_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle)
{
    lv_init();
    ESP_ERROR_CHECK(tick_init());

    lv_disp_t *disp = display_init(lcd_handle);
    if(!disp) return ESP_FAIL;

    if(tp_handle) {
        if(!indev_init(tp_handle)) return ESP_FAIL;
    }

    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    if(!lvgl_mux) return ESP_FAIL;

    BaseType_t core_id = (LVGL_PORT_TASK_CORE < 0) ? tskNO_AFFINITY : LVGL_PORT_TASK_CORE;
    if(xTaskCreatePinnedToCore(lvgl_port_task, "lvgl", LVGL_PORT_TASK_STACK_SIZE, NULL,
                               LVGL_PORT_TASK_PRIORITY, &lvgl_task_handle,
                               core_id) != pdPASS) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

/* Mutex */
bool lvgl_port_lock(int timeout_ms)
{
    if(!lvgl_mux) return false;
    TickType_t t = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mux, t) == pdTRUE;
}
void lvgl_port_unlock(void)
{
    if(lvgl_mux) xSemaphoreGiveRecursive(lvgl_mux);
}

/* VSYNC: rotation des buffers pour triple buffering */
bool lvgl_port_notify_rgb_vsync(void)
{
#if LVGL_PORT_FULL_REFRESH && (LVGL_PORT_LCD_RGB_BUFFER_NUMS == 3) && (EXAMPLE_LVGL_PORT_ROTATION_DEGREE == 0)
    if (lvgl_port_rgb_next_buf != lvgl_port_rgb_last_buf) {
        lvgl_port_flush_next_buf = lvgl_port_rgb_last_buf;
        lvgl_port_rgb_last_buf = lvgl_port_rgb_next_buf;
    }
    return false;
#else
    return false;
#endif
}

// #include "freertos/FreeRTOS.h"
// #include "freertos/semphr.h"
// #include "freertos/task.h"
// #include "esp_lcd_panel_ops.h"
// #include "esp_lcd_panel_rgb.h"
// #include "esp_timer.h"
// #include "esp_log.h"
// #include "lvgl_port.h"

// #define LVGL_PORT_H_RES (EXAMPLE_LCD_H_RES)
// #define LVGL_PORT_V_RES (EXAMPLE_LCD_V_RES)

// static const char *TAG = "lv_port";
// static SemaphoreHandle_t lvgl_mux = NULL;
// static TaskHandle_t lvgl_task_handle = NULL;

// /* Flush direct mode simple */
// static void flush_callback(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
// {
//     esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)drv->user_data;

//     if (lv_disp_flush_is_last(drv)) {
//         esp_lcd_panel_draw_bitmap(panel,
//                                   area->x1, area->y1,
//                                   area->x2 + 1, area->y2 + 1,
//                                   color_map);
//         /* Attente VSYNC pour éviter incohérences visuelles */
//         ulTaskNotifyValueClear(NULL, ULONG_MAX);
//         ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     }
//     lv_disp_flush_ready(drv);
// }

// static lv_disp_t *display_init(esp_lcd_panel_handle_t panel_handle)
// {
//     assert(panel_handle);
//     static lv_disp_draw_buf_t disp_buf;
//     static lv_disp_drv_t disp_drv;

//     void *buf1 = NULL;
//     void *buf2 = NULL;
//     int pixels = LVGL_PORT_H_RES * LVGL_PORT_V_RES;

//     ESP_ERROR_CHECK(esp_lcd_rgb_panel_get_frame_buffer(panel_handle, 2, &buf1, &buf2));
//     lv_disp_draw_buf_init(&disp_buf, buf1, buf2, pixels);

//     lv_disp_drv_init(&disp_drv);
//     disp_drv.hor_res   = LVGL_PORT_H_RES;
//     disp_drv.ver_res   = LVGL_PORT_V_RES;
//     disp_drv.flush_cb  = flush_callback;
//     disp_drv.draw_buf  = &disp_buf;
//     disp_drv.user_data = panel_handle;
//     disp_drv.direct_mode = 1;

//     return lv_disp_drv_register(&disp_drv);
// }

// static void touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
// {
//     esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)indev_drv->user_data;
//     if(!tp){ data->state = LV_INDEV_STATE_RELEASED; return; }
//     uint16_t x,y; uint8_t cnt=0;
//     esp_lcd_touch_read_data(tp);
//     bool pressed = esp_lcd_touch_get_coordinates(tp, &x, &y, NULL, &cnt, 1);
//     if (pressed && cnt>0) {
//         data->point.x = x;
//         data->point.y = y;
//         data->state = LV_INDEV_STATE_PRESSED;
//     } else {
//         data->state = LV_INDEV_STATE_RELEASED;
//     }
// }

// static lv_indev_t *indev_init(esp_lcd_touch_handle_t tp)
// {
//     if(!tp) return NULL;
//     static lv_indev_drv_t indev_drv;
//     lv_indev_drv_init(&indev_drv);
//     indev_drv.type = LV_INDEV_TYPE_POINTER;
//     indev_drv.read_cb = touchpad_read;
//     indev_drv.user_data = tp;
//     return lv_indev_drv_register(&indev_drv);
// }

// static void tick_increment(void *arg){ lv_tick_inc(LVGL_PORT_TICK_PERIOD_MS); }
// static esp_err_t tick_init(void)
// {
//     const esp_timer_create_args_t args = { .callback = &tick_increment, .name = "lv_tick" };
//     esp_timer_handle_t h = NULL;
//     ESP_ERROR_CHECK(esp_timer_create(&args, &h));
//     return esp_timer_start_periodic(h, LVGL_PORT_TICK_PERIOD_MS * 1000);
// }

// static void lvgl_port_task(void *arg)
// {
//     uint32_t wait_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
//     while(true) {
//         if (lvgl_port_lock(-1)) {
//             wait_ms = lv_timer_handler();
//             lvgl_port_unlock();
//         }
//         if (wait_ms > LVGL_PORT_TASK_MAX_DELAY_MS) wait_ms = LVGL_PORT_TASK_MAX_DELAY_MS;
//         if (wait_ms < LVGL_PORT_TASK_MIN_DELAY_MS) wait_ms = LVGL_PORT_TASK_MIN_DELAY_MS;
//         vTaskDelay(pdMS_TO_TICKS(wait_ms));
//     }
// }

// esp_err_t lvgl_port_init(esp_lcd_panel_handle_t lcd_handle, esp_lcd_touch_handle_t tp_handle)
// {
//     lv_init();
//     ESP_ERROR_CHECK(tick_init());

//     lv_disp_t *disp = display_init(lcd_handle);
//     assert(disp);

//     if (tp_handle) {
//         lv_indev_t *indev = indev_init(tp_handle);
//         assert(indev);
//     }

//     lvgl_mux = xSemaphoreCreateRecursiveMutex();
//     assert(lvgl_mux);

//     BaseType_t core_id = (LVGL_PORT_TASK_CORE < 0) ? tskNO_AFFINITY : LVGL_PORT_TASK_CORE;
//     if(xTaskCreatePinnedToCore(lvgl_port_task, "lvgl",
//                                LVGL_PORT_TASK_STACK_SIZE, NULL,
//                                LVGL_PORT_TASK_PRIORITY, &lvgl_task_handle,
//                                core_id) != pdPASS) {
//         return ESP_FAIL;
//     }
//     return ESP_OK;
// }

// bool lvgl_port_lock(int timeout_ms)
// {
//     if(!lvgl_mux) return false;
//     TickType_t t = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
//     return xSemaphoreTakeRecursive(lvgl_mux, t) == pdTRUE;
// }

// void lvgl_port_unlock(void)
// {
//     if(lvgl_mux) xSemaphoreGiveRecursive(lvgl_mux);
// }

// bool lvgl_port_notify_rgb_vsync(void)
// {
//     BaseType_t yield = pdFALSE;
//     xTaskNotifyFromISR(lvgl_task_handle, ULONG_MAX, eNoAction, &yield);
//     return yield == pdTRUE;
// }