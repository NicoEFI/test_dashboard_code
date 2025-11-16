#include "ui/battery.h"

#include <Arduino.h>
#include "lvgl.h"
#include "io_extension.h"

// UI element local au module
static lv_obj_t *BAT_Label = NULL;

// Buffer pour la chaîne affichée (même nom/contenu fonctionnel que dans l'original)
static char bat_v[20] = {0};

// Dernière valeur mesurée
static float last_voltage = 0.0f;

/* Paramètres (fixes comme demandé) */
static const uint8_t SAMPLE_COUNT = 10;
static const uint32_t SAMPLE_INTERVAL_MS = 20;    // intervalle entre échantillons
static const uint32_t DISPLAY_INTERVAL_MS = 1000; // intervalle entre mises à jour d'affichage

/* État simple pour l'acquisition non bloquante */
static bool sampling = false;
static uint8_t sample_idx = 0;
static uint32_t sum_samples = 0;
static uint32_t last_sample_ms = 0;
static uint32_t last_display_ms = 0;

/* Callback LVGL one-shot (identique au code d'origine) */
static void bat_cb(lv_timer_t * timer)
{
    (void)timer;
    if (BAT_Label) {
        lv_label_set_text(BAT_Label, bat_v); // Update the battery voltage label with latest value
    }
}

/* Crée le label batterie (doit être appelé sous lvgl_port_lock) */
void battery_init_ui(void)
{
    BAT_Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(BAT_Label, LV_SIZE_CONTENT);
    lv_obj_set_height(BAT_Label, LV_SIZE_CONTENT);
    lv_obj_center(BAT_Label);
    lv_obj_set_y(BAT_Label, 30);
    lv_label_set_text(BAT_Label, "BAT:3.7V"); // Initial placeholder text

    // Style identique à l'original
    lv_obj_set_style_text_color(BAT_Label, lv_color_hex(0xFFA500), LV_PART_MAIN); // Orange text
    lv_obj_set_style_text_opa(BAT_Label, 255, LV_PART_MAIN);                     // Full opacity
    lv_obj_set_style_text_font(BAT_Label, &lv_font_montserrat_44, LV_PART_MAIN); // Large font

    // initialisation des timers/états
    sampling = false;
    sample_idx = 0;
    sum_samples = 0;
    last_sample_ms = millis();
    last_display_ms = 0; // permet de démarrer immédiatement
}

/*
 * battery_update() : appeler fréquemment depuis loop().
 *
 * - Toutes les DISPLAY_INTERVAL_MS démarrent une acquisition de SAMPLE_COUNT échantillons espacés de SAMPLE_INTERVAL_MS.
 * - On prend le premier échantillon immédiatement lors du démarrage de l'acquisition (comportement proche de l'original).
 * - Après collecte, on calcule la moyenne, conversion en volts, snprintf dans bat_v et on crée le timer LVGL one-shot (100 ms) pour mettre à jour le label.
 */
void battery_update(void)
{
    uint32_t now = millis();

    // Si on n'est pas en train d'acquérir, vérifier si on doit démarrer une nouvelle acquisition
    if (!sampling) {
        if ((now - last_display_ms) >= DISPLAY_INTERVAL_MS) {
            // démarrer acquisition
            sampling = true;
            sample_idx = 0;
            sum_samples = 0;
            // prendre immédiatement le premier échantillon
            uint16_t raw = IO_EXTENSION_Adc_Input();
            sum_samples += raw;
            sample_idx = 1;
            last_sample_ms = now;
        }
        return;
    }

    // acquisition en cours : prendre les échantillons espacés
    if (sampling && sample_idx < SAMPLE_COUNT) {
        if ((now - last_sample_ms) >= SAMPLE_INTERVAL_MS) {
            uint16_t raw = IO_EXTENSION_Adc_Input();
            sum_samples += raw;
            sample_idx++;
            last_sample_ms = now;
        }
    }

    // si on a collecté tous les échantillons, calculer et afficher
    if (sampling && sample_idx >= SAMPLE_COUNT) {
        // calcul moyenne
        float avg = (float)sum_samples / (float)SAMPLE_COUNT;

        // conversion ADC -> tension (10 bits, Vref 3.3V, pont 3:1)
        float voltage = avg * 3.0f * 3.3f / 1023.0f;
    
        // format identique à l'original
        snprintf(bat_v, sizeof(bat_v), "BAT : %0.2fV", voltage);
        last_voltage = voltage;

        // création du timer LVGL one-shot (comme avant)
        lv_timer_t *t = lv_timer_create(bat_cb, 100, NULL); // Trigger after 100 ms
        lv_timer_set_repeat_count(t, 1); // Run only once

        // reset état et mise à jour du timestamp d'affichage
        sampling = false;
        sample_idx = 0;
        sum_samples = 0;
        last_display_ms = now;
    }
}

float battery_get_last_voltage(void)
{
    return last_voltage;
}