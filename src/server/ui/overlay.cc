#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <sys/time.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <ced.h>
#include <ced_menu.h>
#include <gl_font.h>

#include "overlay.h"
#include "draw/layers.h"

using namespace std;

// Owned by glced.cc.
extern int animation_start_time;
extern int animate_layer;
extern float WORLD_SIZE;
int isLayerVisible(int x);

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

void printFPS(void){
    //calculate fps:
    //----------------------
    static int fps=0;
    static int old_fps=0;
    static double startTime;
    struct timeval tv;


    if(setting.fps == false){
        return;
    }


    gettimeofday(&tv, 0);

    if(tv.tv_sec+tv.tv_usec/1000000.0-startTime < 1.0){
        fps++;
    }else{
        startTime=tv.tv_sec+tv.tv_usec/1000000.0;
        //printf("FPS: %i\n", fps);
        old_fps=fps;
        fps=1;
    }

    //print on screen:
    //----------------------

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

    int  world_size=1000; //static worldsize maybe will get problems in the future...
    glOrtho(-world_size*w/h,world_size*w/h,-world_size,world_size, -15*world_size,15*world_size);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    char text[400];

    sprintf(text, "FPS: %i", old_fps);

    glLoadIdentity();

    double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0;
    glColor3f(dark,dark,dark);


    //void *font=GLUT_BITMAP_TIMES_ROMAN_10; //default font
    //glRasterPos2f(-1200,-950);
    //char *c;
    //for (c=text; *c != '\0'; c++) {
    //    glutBitmapCharacter(font, *c);
    //}

    font_render(setting.font, -1200, -950, text);

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

std::string truncateTo(std::string str, size_t max_len) {
  if (str.size() >= max_len) {
      auto truncStr =  str.substr(0, max_len);
      truncStr[max_len-3] = '.';
      truncStr[max_len-2] = '.';
      truncStr[max_len-1] = '.';
      return truncStr;
  }
  return str;
}

static std::string formatShortcut(int iLayer, const char key, const char *description,
                           size_t max_len) {
  std::stringstream sstr;
  sstr << "(" << (isLayerVisible(iLayer) ? "X" : "_") << ") [" << key << "] "
       << std::setfill('0') << std::setw(2) << iLayer << ": " << description;

  return truncateTo(sstr.str(), max_len);
}
void printEventTime(void){
    if( animate_layer < 0 ) return;

    //calculate event time:
    float elapsed_time = 0.001*( (int)SDL_GetTicks() - animation_start_time); // in seconds, but physicswise should be in ns
    char text[42];
    sprintf(text, "Event time: %.3f ns", elapsed_time);
    double dark = 1.-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2]) / 3.0;

    //print on screen:
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    SDL_Rect _disp_r;
    SDL_GetDisplayBounds(0, &_disp_r);
    GLfloat w=(GLfloat)_disp_r.w;
    GLfloat h=(GLfloat)_disp_r.h;
    glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(dark,dark,dark);
    font_render(setting.font, -600, -950, text);


    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void printShortcuts(void){

    const unsigned int MAX_STR_LEN=30;
    int i;

    int height = font_get_height(setting.font) + 2;
    int width  = font_get_width(setting.font, "A");

    //float line = 12; //height of one line
    //float column = MAX_STR_LEN*5; //width of one line

    float line = height; //height of one line
    float column = MAX_STR_LEN*width; //width of one line




    vector<string> shortcuts;
    shortcuts.push_back( "GENERAL SHORTCUTS:" );


    shortcuts.push_back( "[ESC] Quit CED" );
    shortcuts.push_back( "[h] Toggle shortcut frame" );
    shortcuts.push_back( "[CTRL+s] Save screenshot" );
    shortcuts.push_back( "[CTRL+z] Undo" );
    shortcuts.push_back( "[r] Reset view" );
    shortcuts.push_back( "[R] Reset CED" );
    shortcuts.push_back( "[f] Front view" );
    shortcuts.push_back( "[s] Side view" );
    shortcuts.push_back( "[F] Front projection" );
    shortcuts.push_back( "[S] Side projection" );
    shortcuts.push_back( "[v] Fisheye projection" );
    shortcuts.push_back( "[b] Change background color" );
    shortcuts.push_back( "[+] Zoom in" );
    shortcuts.push_back( "[-] Zoom out" );
    shortcuts.push_back( "[c] Center" );
    shortcuts.push_back( "[Z] Cut in z-axe direction" );
    shortcuts.push_back( "[z] Cut in -z-axe direction" );
    shortcuts.push_back( "[>] Increase transparency" );
    shortcuts.push_back( "[<] Decrease transparency" );
    shortcuts.push_back( "[m] Increase detector cut angle" );
    shortcuts.push_back( "[m] Decrease detector cut angle" );
    shortcuts.push_back( "[->] Move in z-direction" );
    shortcuts.push_back( "[<-] Move in -z-direction" );
    shortcuts.push_back( "[`] Toggle all data layers" );
    shortcuts.push_back( "[~] Toggle all detector layers" );


    shortcuts.push_back( "  " );
    shortcuts.push_back( "DATA LAYERS:" );


    for(i=0;i<NUMBER_DATA_LAYER;i++){
        shortcuts.emplace_back(formatShortcut(i, layer_keys[i], layerDescription[i], MAX_STR_LEN));
    }

    shortcuts.push_back( " " );
    shortcuts.push_back( "DETECTOR LAYERS: " );

    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
        shortcuts.emplace_back(formatShortcut(i, detec_layer_keys[-1 * NUMBER_DATA_LAYER + i], layerDescription[i], MAX_STR_LEN));
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();


    glMatrixMode(GL_PROJECTION);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);


    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLfloat w = window_width;
    GLfloat h = window_height;

    int  world_size=1000; //static worldsize maybe will get problems in the future...

    //glOrtho(0,w,h, 0,0,15*WORLD_SIZE);

    //glOrtho(0,w,h,-10,0,15*WORLD_SIZE);

    glOrtho(0,w,h,-1*height,0,15*world_size);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    double border_factor_line=0.005;
    double border_factor_quad=0.0052;


    double boarder_quad = 1000*border_factor_quad;
    double boarder_line = 1000*border_factor_line;


    if(int(w/column) > 1){
        h=(boarder_quad*2.+(shortcuts.size()*1./int((w-3.*boarder_quad)/column) + 1.)*line)*3.+5;
    }else{
        h*=3;
    }

    //glColor4f(HELP_FRAME_FILL_COLOR);

    if((setting.bgcolor[0] + setting.bgcolor[1] + setting.bgcolor[2]) < 0.5*3){
        glColor4f(0.1,0.1,0.1,0.5);
    }else{
        glColor4f(0.9,0.9,0.9,0.5);
    }


    const int ITEMS_PER_COLUMN=int((h/3.0-boarder_quad*2)/(line)); //how many lines per column?
    glBegin(GL_QUADS);
    glVertex3f(boarder_quad, boarder_quad,0);
    glVertex3f(w-boarder_quad,boarder_quad,0);
    glVertex3f(w-boarder_quad, h/3.-boarder_quad,0);
    glVertex3f(boarder_quad, h/3.-boarder_quad,0);
    glEnd();



    //glColor4f(HELP_FRAME_BOARDER_COLOR);
    if((setting.bgcolor[0] + setting.bgcolor[1] + setting.bgcolor[2]) < 0.5*3){
        glColor4f(0.2,0.2,0.2,0.5);
    }else{
        glColor4f(0.8,0.8,0.8,0.5);
    }

    glLineWidth(HELP_FRAME_BOARDER_LINE_SIZE);
    glBegin(GL_LINES);
    glVertex3f(boarder_line, boarder_line,0);
    glVertex3f(w-boarder_line,boarder_line,0);


    glVertex3f(w-boarder_line, h/3-boarder_line,0);
    glVertex3f(boarder_line, h/3.-boarder_line,0);

    glVertex3f(boarder_line, boarder_line,0);
    glVertex3f(boarder_line, h/3. - boarder_line,0);

    glVertex3f(w-boarder_line,boarder_line,0);
    glVertex3f(w-boarder_line, h/3.-boarder_line,0);
    glEnd();

    //glColor3f(HELP_FRAME_TEXT_COLOR);
    if((setting.bgcolor[0] + setting.bgcolor[1] + setting.bgcolor[2]) < 0.5*3){
        glColor3f(1,1,1);
    }else{
        glColor3f(0,0,0);
    }





    for(i=0;(unsigned) i<shortcuts.size();i++){
       font_render(setting.font, int(i/ITEMS_PER_COLUMN)*column+boarder_quad+5, (i%ITEMS_PER_COLUMN)*line+boarder_quad+10, shortcuts[i].c_str());
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

void draw_ced_title_bar(void){
    GLfloat w = window_width;
    GLfloat h = window_height;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, w, h, 0, 0, 15000); // Define the projection matrix

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST); // Turn off depth testing, so graphic renders on top of the 3D scene

    glColor3f(0.20f, 0.20f, 0.20f); // Set color to dark gray
    glBegin(GL_QUADS); // Draw the header rectangle
        glVertex3f(0, 0, 0);
        glVertex3f(0, CED_TITLE_BAR_HEIGHT, 0);
        glVertex3f(w, CED_TITLE_BAR_HEIGHT, 0);
        glVertex3f(w, 0, 0);
    glEnd();

    glColor3f(0.80f, 0.80f, 0.80f); // Set color to light gray
    font_render(setting.font, 6, 3, "C Event Display (CED)"); // Draw the header title

    glEnable(GL_DEPTH_TEST); // Turn on depth testing, so the 3D scene renders normally on the next frame

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
