#include <fg_geometry.h>

#include "render.h"

/*
 * GeoCylinder
 */
void ced_draw_geocylinder_r(CED_GeoCylinderR *c){
    //FIXME: implement fisheye here as well
    //Non trivial due to possible rotations...
    if(!IS_VISIBLE(c->layer)){
        return;
    }

    glLineWidth(1.);
    ced_color(c->color);
  
    glPushMatrix();
  
    glTranslated(c->center[0],c->center[1],c->center[2]);
    
    glRotated(c->rotate[2], 0.0, 0.0, 1.0);
    glRotated(c->rotate[1], 0.0, 1.0, 0.0);
    glRotated(c->rotate[0], 1.0, 0.0, 0.0);
    
    // center!
    glTranslated(0.0,0.0,-(c->z)/2);

    glEnable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    geoSolidCylinder(c->d, c->z, c->sides, 1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  	//glDisable(GL_BLEND);
    glEnd();
  	
    glPopMatrix();
}
