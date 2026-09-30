#include "render.h"

/*
 * GeoBox
 */

void ced_draw_geobox(CED_GeoBox * box )  {
    // a box has 8 vertices, four belonging to the first surface facing
    // the beam, the other four from the second surface
    const unsigned int nPoint = 4;
    const unsigned int nDim   = 3;
    const unsigned int nFace  = 2;
    double face[nFace][nPoint][nDim];
    //  unsigned int iDim, iPoint, iFace;
    unsigned int i, j;
  
  
#if 1
    ced_color(box->color);

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH,GL_NICEST);
    glDepthMask(GL_FALSE);

    glLineWidth(2.);
#endif  

    face[0][0][0] = box->center[0] + (0.5 * box->sizes[0]);
    face[0][0][1] = box->center[1] + (0.5 * box->sizes[1]);
    face[0][0][2] = box->center[2] - (0.5 * box->sizes[2]);
  
    face[0][1][0] = box->center[0] + (0.5 * box->sizes[0]);
    face[0][1][1] = box->center[1] - (0.5 * box->sizes[1]);
    face[0][1][2] = box->center[2] - (0.5 * box->sizes[2]);
  
    face[0][2][0] = box->center[0] - (0.5 * box->sizes[0]);
    face[0][2][1] = box->center[1] - (0.5 * box->sizes[1]);
    face[0][2][2] = box->center[2] - (0.5 * box->sizes[2]);
  
    face[0][3][0] = box->center[0] - (0.5 * box->sizes[0]);
    face[0][3][1] = box->center[1] + (0.5 * box->sizes[1]);
    face[0][3][2] = box->center[2] - (0.5 * box->sizes[2]);    
  
    face[1][0][0] = box->center[0] + (0.5 * box->sizes[0]);
    face[1][0][1] = box->center[1] + (0.5 * box->sizes[1]);
    face[1][0][2] = box->center[2] + (0.5 * box->sizes[2]);
  
    face[1][1][0] = box->center[0] + (0.5 * box->sizes[0]);
    face[1][1][1] = box->center[1] - (0.5 * box->sizes[1]);
    face[1][1][2] = box->center[2] + (0.5 * box->sizes[2]);
  
    face[1][2][0] = box->center[0] - (0.5 * box->sizes[0]);
    face[1][2][1] = box->center[1] - (0.5 * box->sizes[1]);
    face[1][2][2] = box->center[2] + (0.5 * box->sizes[2]);
  
    face[1][3][0] = box->center[0] - (0.5 * box->sizes[0]);
    face[1][3][1] = box->center[1] + (0.5 * box->sizes[1]);
    face[1][3][2] = box->center[2] + (0.5 * box->sizes[2]);
 

    glBegin(GL_LINES);
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

}
