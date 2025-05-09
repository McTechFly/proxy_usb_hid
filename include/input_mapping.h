#ifndef INPUT_MAPPING_H
#define INPUT_MAPPING_H

#include <linux/input.h>
#include <limits.h>
#include <stdbool.h>

// On s'assure que KEY_MAX est défini (normalement dans <linux/input.h>)
#ifndef KEY_MAX
#define KEY_MAX 0x2ff
#endif

typedef struct InputDevice {
    char path[256];                    // Chemin du périphérique (ex. "/dev/input/eventX")
    int fd;                            // Descripteur
    char name[256];                    // Nom du périphérique
    struct input_absinfo absinfo[ABS_CNT]; // Infos des axes
    int has_abs[ABS_CNT];              // Indique si l'axe est présent
    int axis_mapping[ABS_CNT];         // Mapping physique vers virtuel
    int axis_dead_zone[ABS_CNT];       // Zone morte pour chaque axe
    int axis_invert[ABS_CNT];          // Inversion de l'axe
    int axis_virtual_joystick[ABS_CNT]; // Joystick virtuel cible (0 ou 1)
    int axis_virtual_axis[ABS_CNT];     // Axe virtuel (0 à 7)
    int button_mapping[KEY_MAX + 1];         // Mapping des boutons
    int button_virtual_joystick[KEY_MAX + 1];  // Joystick virtuel pour les boutons
    int has_button[KEY_MAX + 1];               // Indique si le bouton existe
    int num_axes;                      // Nombre d'axes détectés
    int num_buttons;                   // Nombre de boutons détectés
    struct input_id id;                // Identifiants du périphérique
} InputDevice;

// Variables globales (définies dans input_mapping.c)
extern InputDevice *g_devices;
extern int g_nb_joysticks;
extern int global_axis_index;
extern int global_button_index;
extern char g_mapping_file[PATH_MAX];

// Prototypes des fonctions de mapping
int parse_hidraw_buttons(const char *hidraw_path, int *button_codes, int max_buttons);
int find_hidraw_for_device(InputDevice *dev, char *hidraw_path, size_t hidraw_path_len);
bool save_mapping(const char *filename, InputDevice *devices, int nb_joysticks, int global_axis, int global_button);
bool load_mapping(const char *filename, InputDevice **devices, int *nb_joysticks, int *global_axis, int *global_button);
void init_physical_devices_wrapper(InputDevice **final_devices, int *nb_final);

#endif // INPUT_MAPPING_H
