#include <fg_geometry.h>

#include "render.h"
#include "../geometry/fisheye.h"

/*
 * GeoCylinder
 */

void ced_draw_geocylinder(CED_GeoCylinder *c){
  
    glPushMatrix();
    glLineWidth(1.);
    ced_color(c->color);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
  
    double transformed_shift = single_fisheye_transform(c->shift, fisheye_alpha);
    glTranslatef(0.0, 0.0, transformed_shift);
    //SM-H: Fisheye code
    double d = single_fisheye_transform(c->d, fisheye_alpha);
  
    double z0 = transformed_shift;
    double z1 = single_fisheye_transform(c->z+c->shift, fisheye_alpha);
    double z = z1-z0;

    if(c->rotate > 0.01 ){
        glRotatef(c->rotate, 0, 0, 1);
    }
    geoSolidCylinder(d, z*2, c->sides, 1);

    glPopMatrix();
}
