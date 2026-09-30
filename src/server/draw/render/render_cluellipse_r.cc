#include "render.h"

/** 
 * Draws 3 orthogonal elipses as wireframes
 */
void ced_draw_cluellipse_r(CED_CluEllipseR * eli )  {
	if(!IS_VISIBLE(eli->layer)){
        return;
    }

  	glPushMatrix();
  	
	/** 1. Ellipsoid centre */
  	glTranslated(eli->center[0],eli->center[1],eli->center[2]);
  	
  	/** 1. Rotate the ellipsoid */
	glRotated(eli->rotate[2], 0.0, 0.0, 1.0);
  	glRotated(eli->rotate[1], 0.0, 1.0, 0.0);
	glRotated(eli->rotate[0], 1.0, 0.0, 0.0);
  	
	/** 1. Set the colour */
	ced_color(eli->color);
	
	/** 1. Case: filled */
	float x,y,z;
	/** ellipsoid 'resolution' */
	float n = 20;
	float t;
	
	/** openGL alpha blending */
	glEnable(GL_BLEND);
    //TODO
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	glBegin(GL_POLYGON);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->radius/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	
	/** 2. Case: unfilled */
	
	//glColor4f(eli->RGBAcolor[0], eli->RGBAcolor[1], eli->RGBAcolor[2], 1.0);
	
	glLineWidth(2.);
	glBegin(GL_LINE_LOOP);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->radius/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	
	/**
	 * 
	 * 
	 * 
	 */
	 
	glRotated(90.0, 0.0, 1.0, 0.0);
	
	/** 1. Set the colour */
	//glColor4f(eli->RGBAcolor[0], eli->RGBAcolor[1], eli->RGBAcolor[2], eli->RGBAcolor[3]);
	
	glBegin(GL_POLYGON);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->height/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	
	/** 2. Case: unfilled */
	
	//glColor4f(eli->RGBAcolor[0], eli->RGBAcolor[1], eli->RGBAcolor[2], 1.0);
	
	glLineWidth(2.);
	glBegin(GL_LINE_LOOP);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->height/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	
 	glRotated(90.0, 1.0, 0.0, 0.0);
	
	/** 1. Set the colour */
	//glColor4f(eli->RGBAcolor[0], eli->RGBAcolor[1], eli->RGBAcolor[2], eli->RGBAcolor[3]);
	
	glBegin(GL_POLYGON);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->height/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	
	/** 2. Case: unfilled */
	
	//glColor4f(eli->RGBAcolor[0], eli->RGBAcolor[1], eli->RGBAcolor[2], 1.0);
	
	glLineWidth(2.);
	glBegin(GL_LINE_LOOP);
	for(t = 0; t < 2*M_PI; t +=2*M_PI/(n) ){
		x = eli->height/2*cos(t);
		y = eli->radius/2*sin(t);
		z = 0;
		glVertex3f(x, y, z);
	}		
	glEnd();
	 

	/** End commands */
    glPopMatrix();
  	glEndList();	
}
