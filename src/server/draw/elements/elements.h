#pragma once

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <math.h>

#include <ced.h>
#include <ced_cli.h>
#include <ced_config.h>

extern CEDsettings setting;
extern double fisheye_alpha;
extern int selected_layer;

#define IS_VISIBLE(x) ((x < (CED_MAX_LAYER-1) && (int)x >= 0)?setting.layer[x]:false)

// Shared helpers
void ced_color(unsigned rgba);

//SM-H: Takes a given point, and returns the fisheye transformed version. Based on transform given in
//'Event display: Can We See What We Want to See',
//H. Drevermann, D. Kuhn, B.S. Nilsson, 1995
//TODO: More elegant (and eficient) implementation possible.
//See http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=262910
CED_Point fisheye_transform(const float x, const float y, const float z, const double scale_factor);

//SM-H: The same as above, but just applied to r or z rather than a whole cartesian co-ordinate system
//CED co-ordinates only defined up to float precision
inline float single_fisheye_transform(float c, const double scale_factor) {
    return c/(1.0+scale_factor*fabs(c));
}

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

// Draw functions
void ced_draw_hit(CED_Hit *h);
void ced_draw_line(CED_Line *h);
void ced_draw_geotube(CED_GeoTube *c);
void ced_draw_geocylinder(CED_GeoCylinder *c);
void ced_draw_geocylinder_r(CED_GeoCylinderR *c);
void ced_draw_ellipsoid_r(CED_EllipsoidR *eli);
void ced_draw_cluellipse_r(CED_CluEllipseR *eli);
void ced_draw_geobox(CED_GeoBox *box);
void ced_draw_geobox_r(CED_GeoBoxR *box);
void ced_draw_cone_r(CED_ConeR *cone);
void ced_draw_geobox_r_solid(CED_GeoBoxR *box);
