#pragma once

#include <ced_config.h>

/*
 * Global display/view state for the glced server
 * (grid vs. surface view, projection, per-layer visibility/cuts, etc).
 */
struct CEDsettings{
    bool trans;         //grid or surface view
    bool persp;         //perspectivic view or flat projection
    bool antia;         //anti aliasing
    bool picking_highlight; //marker at picking position
    double detector_trans[NUMBER_DETECTOR_LAYER];
    double detector_cut_angle[NUMBER_DETECTOR_LAYER];
    double detector_cut_z[NUMBER_DETECTOR_LAYER];
    bool detector_picking;
    bool layer[CED_MAX_LAYER];
    bool phi_projection;
    bool z_projection;
    double view[3];
    double va; //vertical angle of view
    double ha; //horionzional angle of view
    bool fixed_view;
    int win_h; //height of the window (pixel)
    int win_w; //wight of the window (pixel)
    double zoom;
    double world_size;
    double bgcolor[4];
    bool show_axes;
    bool fps;
    int font; //size of text (menu, shortcuts, text in ced window)
};

extern int animation_start_time; // in ms
extern int animate_layer;
