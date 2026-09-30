#include <SDL3/SDL.h>

#include <cstdio>
#include <cstring>

#include "elements.h"
#include "../../third_party/gl_font.h"

/**
 * Draws the energy spectrum legend
 * @author: SD
 * @date: 1.09.09
 * */
void ced_draw_legend(CED_Legend *legend){
    //saves the matrices on the stack
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    //changes the matrices to be compatible with the old ced_draw_legend code:
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    SDL_Rect display_bounds;
    SDL_GetDisplayBounds(SDL_GetPrimaryDisplay(), &display_bounds);
    GLfloat w = (GLfloat)display_bounds.w;
    GLfloat h = (GLfloat)display_bounds.h;

    int  WORLD_SIZE=1000; //static worldsize maybe will get problems in the future...
    glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    //begin original code:

	int color_steps = legend->color_steps;
	float ene_max = legend->ene_max;
	float ene_min = legend->ene_min;
	unsigned int ticks = legend->ticks;
	char scale = legend->scale;
	++ticks; // incremented so that input value is only the number of 'middle ticks'

	/*
	 * The legend position, width and height */
	float legendThickness = 20;
	float stripeThickness = 512/(float)color_steps;
	float x_min = 1100;
	float x_max = x_min+legendThickness;
	float y_min = 400;
	float y_max = y_min+stripeThickness;

	int tickNumber = 1; // 'middle' tick counter
	int i;

	/** ticks */
	char string[6];
	int x_offset = 34;
	int y_offset = 5;
	float num;

	/** Legend header */
	char header [] = "GeV";
	char footer [] = "LOG";
	int x_offset_legend = 60;
	int y_offset_legend = 20;

	int font = setting.font;
  	int tick_size = 10;

	/**
	 *  Legend header: GeV */
	//glColor3f(1.0,1.0,1.0);
    double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0; //ever readable color
    glColor3f(dark,dark,dark);

	font_render(font, x_min-x_offset_legend, y_min+stripeThickness*color_steps-y_offset_legend, header);
	glEnd();
	//glPopMatrix();

	/**
	 *  Legend footer: LOG or LIN */
	switch(scale){
		case 'a': default:
			font_render(font, x_min-x_offset_legend, y_min-y_offset_legend, footer);
			glEnd();
		break;
		/** LIN */
		case 'b':
			strncpy( footer, "LIN", 4 );
			font_render(font, x_min-x_offset_legend, y_min-y_offset_legend, footer);
			glEnd();
		break;
	}

	for (i=0; i<color_steps; ++i) {
		/** This draws the colour spectrum */
		glColor3f(legend->rgb_matrix[i][0]/(float)color_steps,legend->rgb_matrix[i][1]/(float)color_steps,legend->rgb_matrix[i][2]/(float)color_steps);

		glBegin(GL_POLYGON);
		glRasterPos2f(x_min, y_min);
		glVertex3f( x_min,y_min+stripeThickness*i,0.0);
		glVertex3f( x_max,y_min+stripeThickness*i,0.0);
		glVertex3f( x_max,y_max+stripeThickness*i,0.0);
		glVertex3f( x_min,y_max+stripeThickness*i,0.0);
		glEnd();

		/**
		 * Legend: Max & min value display */
		if (i==0 || i==(color_steps-1)){
			glBegin(GL_POLYGON);
			glColor3f(1.0, 1.0, 1.0);
			glRasterPos2f(x_min, y_min);
			glVertex3f( x_max,y_min+stripeThickness*i,0.0);
			glVertex3f( x_max+tick_size,y_min+stripeThickness*i,0.0);
			glVertex3f( x_max+tick_size,y_max+stripeThickness*i,0.0);
			glVertex3f( x_max,y_max+stripeThickness*i,0.0);
			glEnd();

			/**
		 	 * Spectrum max & min value display */
			//glColor3f(1.0f,1.0f,1.0f);
            //double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0; //ever readable color
            glColor3f(dark,dark,dark);



			if (i==0){
				snprintf(string, 6,  "%.1f", ene_min);
				font_render(font, x_min+x_offset, y_min+y_offset, string);
			}
			else if (i==(color_steps-1)){
				//printf("top\n");
				snprintf(string, 6, "%.1f", ene_max);
				font_render(font, x_min+x_offset, y_min+stripeThickness*i+y_offset, string);
            }
		}

		/**
		 *  Legend: middle ticks */
		else if ((i%((color_steps-1)/ticks))==0 && (unsigned)tickNumber<ticks){

			//printf("middle\n");

			float pos;
			pos = (float)tickNumber*(float)color_steps/(float)ticks;

			glBegin(GL_POLYGON);
			glColor3f(1.0, 1.0, 1.0);
			glRasterPos2f(x_min, y_min);
			glVertex3f( x_max,y_min+stripeThickness*pos,0.0);
			glVertex3f( x_max+tick_size,y_min+stripeThickness*pos,0.0);
			glVertex3f( x_max+tick_size,y_max+stripeThickness*pos,0.0);
			glVertex3f( x_max,y_max+stripeThickness*pos,0.0);
			glEnd();

			/** Mid-tick legend generation: LOG */
			switch(scale){
				case 'a': default:
					num = pow( (ene_max +1)/(ene_min +1), (float)tickNumber/(float)ticks ) * (ene_min+1) - 1;
				break;
				/** LIN */
				case 'b':
					num = (((ene_max-ene_min)/ticks)*tickNumber) + ene_min;
				break;
			}

			snprintf(string, 6, "%.1f", num);


            glColor3f(dark,dark,dark);
			font_render(font, x_min+x_offset, y_min+stripeThickness*pos+y_offset, string);

			++tickNumber;
		}
	}
	glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
