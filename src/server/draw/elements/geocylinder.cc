#include "../../third_party/fg_geometry.h"

#include "elements.h"

/*
 * GeoCylinder
 */

void ced_draw_geocylinder(CED_GeoCylinder *c){
  
    glPushMatrix();
    glLineWidth(1.);
    ced_color(c->color);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
  
    glTranslatef(0.0, 0.0, c->shift);
    double d = c->d;
    double z = c->z;

    if(c->rotate > 0.01 ){
        glRotatef(c->rotate, 0, 0, 1);
    }
    geoSolidCylinder(d, z*2, c->sides, 1);

    glPopMatrix();
}
