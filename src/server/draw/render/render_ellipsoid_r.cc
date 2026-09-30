#include <fg_geometry.h>

#include "render.h"

/** Draws an ellipsoid 
 * Code based on http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=242991
 * */

void ced_draw_ellipsoid_r(CED_EllipsoidR * eli )  {
	if(!IS_VISIBLE(eli->layer))
      return;

	/** ellipsoid 'resolution' */
	int slices = 10;
	int stacks = 10;

  	glPushMatrix();

	/** Ellipsoid centre */
  	glTranslated(eli->center[0],eli->center[1],eli->center[2]);
  	
  	/** Rotate the ellipsoid */
	glRotated(eli->rotate[2], 0.0, 0.0, 1.0);
  	glRotated(eli->rotate[1], 0.0, 1.0, 0.0);
  	glRotated(eli->rotate[0], 1.0, 0.0, 0.0);
  	
    /** Quadric object */
   	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /** Set polygon's filling */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/** Set the colour */
	ced_color(eli->color);

	/** Draw the cone */
	glEnable(GL_BLEND);

	/** Alter scale factors so as to obtain an ellipsoid from a sphere */
	glScaled(eli->size[0]/2, eli->size[1]/2, eli->size[2]/2);

	geoSolidSphere(1.0, slices, stacks);
    glPopMatrix();
  	glEndList();	
}
