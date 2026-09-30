#pragma once

// Draw a partial cylinder made of lines (for the detector geometry)
void drawPartialLineCylinder(
    double length,
    double R /*radius*/,
    double iR /*inner radius*/,
    int edges,
    double angle_cut_off,
    double angle_cut_off_left,
    bool outer_face=1,
    bool inner_face=1
);

// Draw a partial cylinder made of planes (for the detector geometry)
void drawPartialCylinder(double length,
    double R /*radius*/,
    double iR /*inner radius*/,
    int edges,
    double angle_cut_off,
    double angle_cut_off_left,
    bool outer_face=1,
    bool inner_face=1,
    double irotate=0
);
