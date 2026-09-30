#include "render.h"

/*
 * GeoBoxRSolid
 */

void ced_draw_geobox_r_solid(CED_GeoBoxR * box )  {

//   std::cout << " ced_draw_geobox_r_solid was called." << std::endl;

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
	
	
	//ced_color(box->color);
	//glLineWidth(2);
	
	glPushMatrix(); // push the matrix onto the matrix stack and pop it off (preserving the original matrix)
	
	glTranslated(box->center[0],box->center[1],box->center[2]);
	
	glRotated(box->rotate[2], 0.0, 0.0, 1.0);
	glRotated(box->rotate[1], 0.0, 1.0, 0.0);
	glRotated(box->rotate[0], 1.0, 0.0, 0.0);
	
	face[0][0][0] =  + (0.5 * box->sizes[0]);
	face[0][0][1] =  + (0.5 * box->sizes[1]);
	face[0][0][2] =  - (0.5 * box->sizes[2]);
	
	face[0][1][0] =  + (0.5 * box->sizes[0]);
	face[0][1][1] =  - (0.5 * box->sizes[1]);
	face[0][1][2] =  - (0.5 * box->sizes[2]);
	
	face[0][2][0] =  - (0.5 * box->sizes[0]);
	face[0][2][1] =  - (0.5 * box->sizes[1]);
	face[0][2][2] =  - (0.5 * box->sizes[2]);
	
	face[0][3][0] =  - (0.5 * box->sizes[0]);
	face[0][3][1] =  + (0.5 * box->sizes[1]);
	face[0][3][2] =  - (0.5 * box->sizes[2]);
	
	face[1][0][0] =  + (0.5 * box->sizes[0]);
	face[1][0][1] =  + (0.5 * box->sizes[1]);
	face[1][0][2] =  + (0.5 * box->sizes[2]);
	
	face[1][1][0] =  + (0.5 * box->sizes[0]);
	face[1][1][1] =  - (0.5 * box->sizes[1]);
	face[1][1][2] =  + (0.5 * box->sizes[2]);
	
	face[1][2][0] =  - (0.5 * box->sizes[0]);
	face[1][2][1] =  - (0.5 * box->sizes[1]);
	face[1][2][2] =  + (0.5 * box->sizes[2]);
	
	face[1][3][0] =  - (0.5 * box->sizes[0]);
	face[1][3][1] =  + (0.5 * box->sizes[1]);
	face[1][3][2] =  + (0.5 * box->sizes[2]);
	

	//  drawing the first face


	//ced_color(box->color);
    double trans=0.5;
    if(box->layer < NUMBER_DETECTOR_LAYER){
       trans=setting.detector_trans[box->layer]; 
    }
    GLfloat face_color[4]={static_cast<GLfloat>(((box->color>>16)&0xff)/255.0),static_cast<GLfloat>(((box->color>>8)&0xff)/255.0),static_cast<GLfloat>(((box->color)&0xff)/255.0), static_cast<GLfloat>(trans)};
    //GLfloat line_color[4]={((c->color>>16)&0xff)/255.0/2.0+(1.0-setting.bgcolor[0])/2.0,((c->color>>8)&0xff)/255.0/2.0+(1.0-setting.bgcolor[1])/2.0,((c->color)&0xff)/255.0/2.0+(1.0-setting.bgcolor[2])/2.0, (1-setting.trans_value)+CED_GEOTUBE_LINE_MAX_TRANS}; //lines in detector color mixed with anti background color
    glColor4f(face_color[0], face_color[1], face_color[2], face_color[3]);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    
    glMatrixMode(GL_MODELVIEW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 
	glBegin(GL_POLYGON);

   // glBegin(GL_QUADS);
	
	glVertex3f( (float) face[0][0][0], (float) face[0][0][1],  (float) face[0][0][2] );
	glVertex3f( (float) face[0][1][0], (float) face[0][1][1],  (float) face[0][1][2] );
	glVertex3f( (float) face[0][2][0], (float) face[0][2][1],  (float) face[0][2][2] );
	glVertex3f( (float) face[0][3][0], (float) face[0][3][1],  (float) face[0][3][2] );
	
	glEnd();

	// drawing the second face
	glBegin(GL_POLYGON);
	
	glVertex3f( (float) face[1][0][0], (float) face[1][0][1],  (float) face[1][0][2] );
	glVertex3f( (float) face[1][1][0], (float) face[1][1][1],  (float) face[1][1][2] );
	glVertex3f( (float) face[1][2][0], (float) face[1][2][1],  (float) face[1][2][2] );
	glVertex3f( (float) face[1][3][0], (float) face[1][3][1],  (float) face[1][3][2] );
	
	glEnd();
	
	// drawing the sides
	glBegin(GL_POLYGON);
	
	glVertex3f( (float) face[0][0][0], (float) face[0][0][1],  (float) face[0][0][2] );
	glVertex3f( (float) face[0][1][0], (float) face[0][1][1],  (float) face[0][1][2] );
	glVertex3f( (float) face[1][1][0], (float) face[1][1][1],  (float) face[1][1][2] );
	glVertex3f( (float) face[1][0][0], (float) face[1][0][1],  (float) face[1][0][2] );
	
	glEnd();
	
	glBegin(GL_POLYGON);
	
	glVertex3f( (float) face[0][1][0], (float) face[0][1][1],  (float) face[0][1][2] );
	glVertex3f( (float) face[0][2][0], (float) face[0][2][1],  (float) face[0][2][2] );
	glVertex3f( (float) face[1][2][0], (float) face[1][2][1],  (float) face[1][2][2] );
	glVertex3f( (float) face[1][1][0], (float) face[1][1][1],  (float) face[1][1][2] );
	
	glEnd();
	
	glBegin(GL_POLYGON);
	
	glVertex3f( (float) face[0][2][0], (float) face[0][2][1],  (float) face[0][2][2] );
	glVertex3f( (float) face[0][3][0], (float) face[0][3][1],  (float) face[0][3][2] );
	glVertex3f( (float) face[1][3][0], (float) face[1][3][1],  (float) face[1][3][2] );
	glVertex3f( (float) face[1][2][0], (float) face[1][2][1],  (float) face[1][2][2] );
	
	glEnd();
	
	glBegin(GL_POLYGON);
	
	glVertex3f( (float) face[0][3][0], (float) face[0][3][1],  (float) face[0][3][2] );
	glVertex3f( (float) face[0][0][0], (float) face[0][0][1],  (float) face[0][0][2] );
	glVertex3f( (float) face[1][0][0], (float) face[1][0][1],  (float) face[1][0][2] );
	glVertex3f( (float) face[1][3][0], (float) face[1][3][1],  (float) face[1][3][2] );
	
	glEnd();
	
	glPopMatrix();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //default
}
