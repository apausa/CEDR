#include <ced.h>
#include <ced_cli.h>

#include "render/render.h"
#include "ui/selection.h"

#define PORT  0x1234

CEDsettings setting;

static unsigned HIT_ID=0;
static unsigned LINE_ID=0;
static unsigned CED_PICKING_TEXT_ID=0;
static unsigned GEOT_ID=0;
static unsigned GEOC_ID=0;
static unsigned GEOCR_ID=0;
static unsigned ELLIPSOID_ID = 0;
static unsigned CLUELLIPSE_ID = 0;
static unsigned TEXT_ID=0;
static unsigned LEGEND_ID=0;
static unsigned GEOB_ID = 0;
static unsigned GEOBR_ID = 0;
static unsigned CONER_ID = 0;
static unsigned GEOBRS_ID = 0;

void ced_register_elements(void){
  //the order of this items is very important!!! 
  //1
  GEOC_ID       =ced_register_element(sizeof(CED_GeoCylinder),(ced_draw_cb)ced_draw_geocylinder);
  //2
  GEOCR_ID      =ced_register_element(sizeof(CED_GeoCylinderR),(ced_draw_cb)ced_draw_geocylinder_r);
  //3
  LINE_ID       =ced_register_element(sizeof(CED_Line),(ced_draw_cb)ced_draw_line);
  //4
  HIT_ID        =ced_register_element(sizeof(CED_Hit),(ced_draw_cb)ced_draw_hit);
  //5
  GEOB_ID       =ced_register_element(sizeof(CED_GeoBox),(ced_draw_cb)ced_draw_geobox);
  //6
  GEOBR_ID      =ced_register_element(sizeof(CED_GeoBoxR),(ced_draw_cb)ced_draw_geobox_r);
  //7
  GEOBRS_ID     =ced_register_element(sizeof(CED_GeoBoxR),(ced_draw_cb)ced_draw_geobox_r_solid);
  //8
  CONER_ID      =ced_register_element(sizeof(CED_ConeR),(ced_draw_cb)ced_draw_cone_r);
  //9
  ELLIPSOID_ID  =ced_register_element(sizeof(CED_EllipsoidR),(ced_draw_cb)ced_draw_ellipsoid_r);
  //10
  CLUELLIPSE_ID =ced_register_element(sizeof(CED_CluEllipseR),(ced_draw_cb)ced_draw_cluellipse_r);
  //11
  TEXT_ID       =ced_register_element(sizeof(CED_TEXT),(ced_draw_cb)print_layer_text); //hauke

  /** due to an issue w/ drawing the legend (in 2D) this has to come last ! */
  //12
  LEGEND_ID  =ced_register_element(sizeof(CED_Legend),(ced_draw_cb)ced_draw_legend);
  //TEXT_ID   =ced_register_element(sizeof(CED_TEXT),(ced_draw_cb)ced_draw_text); //hauke
  //LAYER_TEXT_ID   =ced_register_element(sizeof(LAYER_TEXT),(ced_draw_cb)print_layer_text); //hauke

  //13
  GEOT_ID       =ced_register_element(sizeof(CED_GeoTube),(ced_draw_cb)ced_draw_geotube);
  //14
  CED_PICKING_TEXT_ID=ced_register_element(sizeof(CED_PICKING_TEXT),(ced_draw_cb)ced_write_picking_text);
}
