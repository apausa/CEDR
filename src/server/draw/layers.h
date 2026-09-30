#pragma once

#include <ced.h>

extern char layerDescription[CED_MAX_LAYER][CED_MAX_LAYER_CHAR];
extern const char layer_keys[];
extern const char detec_layer_keys[];

void addLayerDescriptionToMenu(int id, char *str);
void print_layer_text(CED_TEXT *obj);
