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
