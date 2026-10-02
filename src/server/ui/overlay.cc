#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <SDL3/SDL.h>

#include <cstdio>
#include <sys/time.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <ced.h>
#include <settings.h>
#include <third_party/gl_font.h>

#include "overlay.h"
#include "layers.h"
#include "utils/helpers.h"

using namespace std;

// Owned by glced.cc.
extern int animation_start_time;
extern int animate_layer;
extern float WORLD_SIZE;
extern GLfloat window_width;
extern GLfloat window_height;
extern CEDsettings setting;

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
    shortcuts.push_back( "[CTRL+z] Undo" );
    shortcuts.push_back( "[r] Reset view" );
    shortcuts.push_back( "[R] Reset CED" );
    shortcuts.push_back( "[f] Front view" );
    shortcuts.push_back( "[s] Side view" );
    shortcuts.push_back( "[F] Front projection" );
    shortcuts.push_back( "[S] Side projection" );
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
