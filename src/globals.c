// globals.c
#include "input_mapping.h" // pour InputDevice *g_devices; ...
// Déclarations extern si besoin
#include <stddef.h>

InputDevice *g_devices = NULL;
int g_nb_joysticks = 0;
int ep_int_in0 = -1;
int ep_int_in1 = -1;

