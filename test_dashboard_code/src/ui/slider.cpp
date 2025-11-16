#include "ui/slider.h"

#include "lvgl.h"
#include "io_extension.h"

// Label pour afficher la valeur du slider (scope fichier)
static lv_obj_t *label = NULL;

// Callback d'événement du slider (identique au code d'origine)
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t *slider = lv_event_get_target(e); // Get the slider that triggered the event

    // Update the label with the slider's current value
    lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));

    // Set the PWM duty cycle based on slider value (inverted: 100 = off, 0 = full brightness)
    uint8_t duty = (100 - lv_slider_get_value(slider));
    // analogWrite(LED_GPIO_PIN, duty * (0xFF / 100.0)); // kept commented as original
    IO_EXTENSION_Pwm_Output(duty); // Optional external PWM controller
}

void lvgl_slider(void)
{
    // Startup call identical to original to ensure identical behavior:
    IO_EXTENSION_Pwm_Output(0); // Optional external control for brightness (startup full brightness)

    // Create the slider widget on the active LVGL screen
    lv_obj_t *slider = lv_slider_create(lv_scr_act());

    // Set slider dimensions and center it on screen
    lv_obj_set_width(slider, 200);
    lv_obj_center(slider);

    // Set initial slider value to 100 (LED off if active-low)
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);

    // Attach the event callback for slider changes
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create a label to display the current slider value
    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "100"); // Initial label value
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); // Position above slider
}