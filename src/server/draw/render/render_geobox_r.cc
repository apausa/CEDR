#include <iostream>

#include "render.h"
#include "../geometry/fisheye.h"

/*
 * GeoBoxR 
 */

void ced_draw_geobox_r(CED_GeoBoxR * box )  {

    std::cout << " ced_draw_geobox_r was called." << std::endl;

    if(!IS_VISIBLE(box->layer)){
        return;
    }

    // a box has 8 vertices, four belonging to the first surface facing
    // the beam, the other four from the second surface
    const unsigned int nPoint = 4;
    const unsigned int nDim   = 3;
    const unsigned int nFace  = 2;
    double face[nFace][nPoint][nDim];
    //  unsigned int iDim, iPoint, iFace;
    unsigned int i, j;
  
    ced_color(box->color);

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH,GL_NICEST);
    glDepthMask(GL_FALSE);

    glLineWidth(2);
  
    glPushMatrix(); // push the matrix onto the matrix stack and pop it off (preserving the original matrix)
    //SM-H: Fisheye transform the radial vector to the centre of the box
    //FIXME: Seems to break the test_ced, but works with sample events??!!
    //Technically also incorrect, in that it only moves the centre of the box, and does not transform the whole thing
    CED_Point center_transformed = fisheye_transform(box->center[0], box->center[1], box->center[2], fisheye_alpha);
    glTranslated(center_transformed.x,center_transformed.y,center_transformed.z);
    
    glRotated(box->rotate[2], 0.0, 0.0, 1.0);
    glRotated(box->rotate[1], 0.0, 1.0, 0.0);
    glRotated(box->rotate[0], 1.0, 0.0, 0.0);
   
    //Deal with z-axis as well, this is given by box-sizes[2]
    //Need half the box size, and also the distance from the axis 
    //since this determines how much the geobox is distorted by
    double z0 = center_transformed.z;
    double z1 = single_fisheye_transform(box->center[2]+box->sizes[2], fisheye_alpha);
    double box_z = z1-z0; 
    face[0][0][0] =  + (0.5 * box->sizes[0]);
    face[0][0][1] =  + (0.5 * box->sizes[1]);
    face[0][0][2] =  - (0.5 * box_z);
  
    face[0][1][0] =  + (0.5 * box->sizes[0]);
    face[0][1][1] =  - (0.5 * box->sizes[1]);
    face[0][1][2] =  - (0.5 * box_z);
  
    face[0][2][0] =  - (0.5 * box->sizes[0]);
    face[0][2][1] =  - (0.5 * box->sizes[1]);
    face[0][2][2] =  - (0.5 * box_z);
  
    face[0][3][0] =  - (0.5 * box->sizes[0]);
    face[0][3][1] =  + (0.5 * box->sizes[1]);
    face[0][3][2] =  - (0.5 * box_z);
  
    face[1][0][0] =  + (0.5 * box->sizes[0]);
    face[1][0][1] =  + (0.5 * box->sizes[1]);
    face[1][0][2] =  + (0.5 * box_z);
  
    face[1][1][0] =  + (0.5 * box->sizes[0]);
    face[1][1][1] =  - (0.5 * box->sizes[1]);
    face[1][1][2] =  + (0.5 * box_z);
  
    face[1][2][0] =  - (0.5 * box->sizes[0]);
    face[1][2][1] =  - (0.5 * box->sizes[1]);
    face[1][2][2] =  + (0.5 * box_z);
  
    face[1][3][0] =  - (0.5 * box->sizes[0]);
    face[1][3][1] =  + (0.5 * box->sizes[1]);
    face[1][3][2] =  + (0.5 * box_z);
  
//    glBegin(GL_LINES);

    glBegin(GL_TRIANGLE_STRIP);
    // drawing the first (i=0) and second (i=1) faces
    for(i = 0; i < 2; i++){
        glVertex3f( (float) face[i][0][0], (float) face[i][0][1],  (float) face[i][0][2] );
        for(j = 1; j < 4; j++){
            glVertex3f((float) face[i][j][0],(float) face[i][j][1],(float) face[i][j][2]);
            glVertex3f((float) face[i][j][0],(float) face[i][j][1],(float) face[i][j][2]);
        }      
        glVertex3f( (float) face[i][0][0], (float) face[i][0][1],  (float) face[i][0][2] );
    }
  
    // drawing the connections
    for(j = 0; j < 4; j++){
        glVertex3f( (float) face[0][j][0], (float) face[0][j][1],  (float) face[0][j][2] );
        glVertex3f( (float) face[1][j][0], (float) face[1][j][1],  (float) face[1][j][2] );
    }
    glEnd();
  

    glDepthMask(GL_TRUE);
    glDisable(GL_LINE_SMOOTH);

    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glPopMatrix();
}
