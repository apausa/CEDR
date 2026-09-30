#include <fg_geometry.h>

#include "render.h"

/**
 * Draws a opaque cone with a custom alpha colour channel.
 * Warning: the cone centre is the vertex (not the centre of teh base!)
 * @author: SD
 * @date: 02.09.09
 * */

void ced_draw_cone_r(CED_ConeR * cone )  {
	if(!IS_VISIBLE(cone->layer)){
        return;
    }

	/** cone size */
	float base = cone->base;
	float height = cone->height;
	/** cone 'resolution' */
	int slices = 16;
	int stacks = 1;

	glMatrixMode(GL_MODELVIEW);
  	glPushMatrix(); // push the matrix onto the matrix stack and pop it off (preserving the original matrix)

	/** Spatial translation */
  	glTranslated(cone->center[0],cone->center[1],cone->center[2]);
  	
  	/** Rotate the cone */
	glRotated(cone->rotate[2], 0.0, 0.0, 1.0);
  	glRotated(cone->rotate[1], 0.0, 1.0, 0.0);
  	glRotated(cone->rotate[0], 1.0, 0.0, 0.0);
  	
  	/** Swap the vertex with the base, so that the cone has a vertex centered at center[] */

	/** Draw the cone */
    //TODO
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(cone->RGBAcolor[0], cone->RGBAcolor[1], cone->RGBAcolor[2], cone->RGBAcolor[3]);


  	glRotated(180, 1.0, 0.0, 0.0);
  	glTranslated(0.0, 0.0, -(cone->height));
	geoSolidCone(base, height, slices, stacks);

    

	//glLineWidth(0.5);
	//glutWireCone(base, height, slices*10, stacks*10);

    //GLUquadricObj *q1 = gluNewQuadric();
    //gluCylinder(q1, 0,base , height, slices, stacks);
    //gluDeleteQuadric(q1);
	glEnd();
	glPopMatrix();
}
