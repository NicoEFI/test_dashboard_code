#ifndef UI_BATTERY_H
#define UI_BATTERY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise uniquement l'UI pour la batterie (création du label).
 * À appeler sous lvgl_port_lock() comme dans le code d'origine.
 */
void battery_init_ui(void);

/**
 * Effectue une itération de mesure/affichage de la batterie.
 * Comportement identique au code original (moyennage 10 échantillons,
 * conversion, sprintf, puis création d'un timer LVGL one-shot appelant
 * une callback interne qui met à jour le label).
 *
 * Appeler depuis loop() (comme dans le code d'origine).
 */
void battery_update(void);

/**
 * Retourne la dernière tension lue (en volts).
 */
float battery_get_last_voltage(void);

#ifdef __cplusplus
}
#endif

#endif // UI_BATTERY_H