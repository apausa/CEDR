/* "C" event display.
 * Communications related part. 
 *
*ik
 * Alexey Zhelezov, DESY/ITEP, 2005 */
#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>

#include <ced.h>
#include <ced_cli.h>
#include <ced_config.h>
#include <SDL3/SDL.h>
#include <third_party/gl_font.h>
#include "third_party/fg_geometry.h"
#include "ui/layers.h"
#include "ui/input.h"
#include "ui/overlay.h"
#include "ui/selection.h"

#include <sys/select.h>

#include <ctype.h>
#include <sys/time.h>
#include <netdb.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>


#include <ced_menu.h>
#include "io/screenshot.h"
#include "ui/menu.h"

#define DEFAULT_WORLD_SIZE 1000.  //SJA:FIXED Reduce world size to give better scale

using namespace std;

//*************** global variables ***************************************//
int animation_start_time = 0;
int animate_layer = -1;
int last_selected_layer;
extern CEDsettings setting;
extern CEDsettings setting_old[5];

static int subSave;
static int subLoad;
int showHelp=0;
float WORLD_SIZE;
float FISHEYE_WORLD_SIZE;
double fisheye_alpha = 0.0;
long int doubleClickTime=0;
static float BG_COLOR[4];
extern int SELECTED_ID ;

//fg - make axe a global to be able to rescale the world volume
static GLfloat axe[][3]={
  { 0., 0., 0., },
  { DEFAULT_WORLD_SIZE/2, 0., 0. },
  { 0., DEFAULT_WORLD_SIZE/2, 0. },
  { 0., 0., DEFAULT_WORLD_SIZE/2 }
};

// allows to reset the visible world size
void set_world_size( float length) {
  WORLD_SIZE = length ;
  axe[1][0] = WORLD_SIZE / 2. ;
  axe[2][1] = WORLD_SIZE / 2. ;
  axe[3][2] = WORLD_SIZE / 2. ;
};

void defaultSettings(void){
    setting.trans=true;
    setting.persp=true;
    setting.antia=false;
    setting.light=false;
    setting.picking_highlight=false;

    setting.win_w=500;
    setting.win_h=500;
    setting.show_axes=true;
    setting.fps=false;

    for(int i=0;i<4;i++){
        setting.bgcolor[i]=1; //white
    }

    setting.font=FONT_M;

    for(int i=0;i<CED_MAX_LAYER;i++){
        setting.layer[i]=true; // turn all layers on
    }

    for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
        setting.detector_trans[i]=0.8;
        setting.detector_cut_angle[i]=0;
        setting.detector_cut_z[i]=7000;
    }

    setting.phi_projection=false;
    setting.z_projection=false;
    setting.fixed_view=false;

    mm=mm_reset;
    fisheye_alpha=0;
    set_world_size(DEFAULT_WORLD_SIZE);

    setting.va=mm.va;
    setting.ha=mm.ha;
    setting.zoom=mm.sf;
    setting.fisheye_alpha=fisheye_alpha;
    setting.fisheye_world_size=FISHEYE_WORLD_SIZE;
    setting.world_size=WORLD_SIZE;

    setting.autoshot=false;
    setting.autoshot_scale=1;

    std::cout << "Set options to default settings" << std::endl;
}

float userDefinedBGColor[] = {-1.0, -1.0, -1.0, -1.0};

extern int socket_fd;
extern void (*socket_fn)(void);
extern bool client_connected;

CED_SubSubMenu *detectorlayermenu;
CED_SubSubMenu *datalayermenu;
CED_PopUpMenu *popupmenu;
CED_Menu *ced_menu=NULL;

bool ced_needs_redraw = false;
SDL_Window* ced_sdl_window = nullptr;
void (*idle_func)(void) = nullptr;

GLfloat window_width = 0.;
GLfloat window_height = 0.;

// ********** function definitions  (rest of file) ************************** //

//set background color (hauke)
void set_bg_color(float one, float two, float three, float four){
    BG_COLOR[0]=one;
    BG_COLOR[1]=two;
    BG_COLOR[2]=three;
    BG_COLOR[3]=four;

    glClearColor(BG_COLOR[0],BG_COLOR[1],BG_COLOR[2],BG_COLOR[3]);
}

int isLayerVisible(int x){
    //return(ced_visible_layers[x]);

    return(setting.layer[x]);
}
static void init(void){
    //Set background color
    glClearColor(BG_COLOR[0],BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);

    //glShadeModel(GL_FLAT);
    glShadeModel(GL_SMOOTH);

    glClearDepth(1);

    glEnable(GL_DEPTH_TEST); //activate 'depth-test'

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear buffers

    //glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //default

    //glEnable(GL_POLYGON_STIPPLE);


    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //glass
    //glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA); //locks nice, but lines diapear

    //glBlendFunc(GL_ONE, GL_ZERO);
    //glBlendFunc(GL_ONE, GL_ONE);
    //glClearColor(0,0,0,0);

    glEnableClientState(GL_VERTEX_ARRAY);
    // GL_NORMAL_ARRAY GL_COLOR_ARRAY GL_TEXTURE_COORD_ARRAY,GL_EDGE_FLAG_ARRAY

    // to make round points
    //glEnable(GL_POINT_SMOOTH);

    // to put text
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    // To enable Alpha channel (expensive !!!)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Point pick_point;
Point pre_pick_point;
int selected_layer;
bool  select_nothing=true;

// bitmaps for X,Y and Z
static unsigned char x_bm[]={
    0xc3,0x42,0x66,0x24,0x24,0x18,
    0x18,0x24,0x24,0x66,0x42,0xc3
};
static unsigned char y_bm[]={
    0xc0,0x40,0x60,0x20,0x30,0x10,
    0x18,0x2c,0x24,0x66,0x42,0xc3
};
static unsigned char z_bm[]={
    0xff,0x40,0x60,0x20,0x30,0x10,
    0x08,0x0c,0x04,0x06,0x02,0xff
};


static void axe_arrow(void){
    GLfloat k=WORLD_SIZE/window_height;
    geoSolidCone(8.*k, 30.*k, 16, 5);
}

static void display_world(void){
    /*   static GLfloat axe[][3]={ */
    /*     { 0., 0., 0., }, */
    /*     { WORLD_SIZE/2, 0., 0. }, */
    /*     { 0., WORLD_SIZE/2, 0. }, */
    /*     { 0., 0., WORLD_SIZE/2 } */
    /*   }; */
    //  unsigned i;
    if(setting.show_axes == false){
        return;
    }


    glColor3f(AXES_COLOR);
    //glLineWidth(2.);
    glLineWidth(AXES_LINE_SIZE);

    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[1]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[2]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3fv(axe[0]);
    glVertex3fv(axe[3]);
    glEnd();

    glColor3f(0.5,0.5,0.8);
    glPushMatrix();
    //glTranslatef(mm.mv.x,mm.mv.y,mm.mv.z);
    glTranslatef(WORLD_SIZE/2.-WORLD_SIZE/100.,0.,0.);
    glRotatef(90.,0.0,1.0,0.0);
    axe_arrow();
    glPopMatrix();

    glPushMatrix();
    //glTranslatef(mm.mv.x,mm.mv.y,mm.mv.z);
    glTranslatef(0.,WORLD_SIZE/2.-WORLD_SIZE/100.,0.);
    glRotatef(-90.,1.0,0.,0.);
    axe_arrow();
    glPopMatrix();


    glPushMatrix();
    //glTranslatef(mm.mv.x,mm.mv.y,mm.mv.z);
    glTranslatef(0.,0.,WORLD_SIZE/2.-WORLD_SIZE/100.);
    axe_arrow();
    glPopMatrix();

    // Draw X,Y,Z ...
    //glColor3f(1.,1.,1.); //white labels
    //glColor3f(0.,0.,0.); //black labels

    glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
    double dark=1.0-(setting.bgcolor[0]+setting.bgcolor[1]+setting.bgcolor[2])/3.0;
    //glColor3f(1-setting.bgcolor[0], 1-setting.bgcolor[1], 1-setting.bgcolor[2]);
    glColor3f(dark,dark,dark);


    glRasterPos3f(WORLD_SIZE/2.+WORLD_SIZE/8,0.,0.);
    glBitmap(8,12,4,6,0,0,x_bm);
    glRasterPos3f(0.,WORLD_SIZE/2.+WORLD_SIZE/8,0.);
    glBitmap(8,12,4,6,0,0,y_bm);
    glRasterPos3f(0.,0.,WORLD_SIZE/2.+WORLD_SIZE/8);
    glBitmap(8,12,4,6,0,0,z_bm);


    //buildMenuPopup(); //hauke: test
    //glutAttachMenu(GLUT_RIGHT_BUTTON);

}

static void display(void){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();

    // TODO: fix it!
    // in case of no rotate, in some cases it could get strange
    // lines in fisheye view from (0,0,0) to (-inf, -inf,x)

    setting.zoom=mm.sf;
    glScalef(mm.sf,mm.sf,mm.sf); //zoom

    glRotatef(mm.va,1.,0.,0.); //rotate
    glRotatef(mm.ha,0.,1.0,0.); //rotate
    glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z); //move

    if(setting.picking_highlight==true && select_nothing == false){
        glColor3f(1,0,0);
        glPointSize(10);
        glBegin(GL_POINTS);
        //cout<< "point: " << pick_point.x << ", " << pick_point.y << ", " << pick_point.z << endl;
        glVertex3f(pick_point.x,pick_point.y,pick_point.z);
        glEnd();
    }
    // draw static objects
    display_world(); //only axes?

    // draw elements (hits + detector)
    ced_prepare_objmap();
    ced_do_draw_event();


    if(showHelp == 1){
        printShortcuts();
    }


    glDisable(GL_LIGHTING);
    draw_ced_title_bar();
    ced_menu->draw();
    popupmenu->draw();
    printFPS();
    printEventTime();

    if(setting.light==true){
        glEnable(GL_LIGHTING);
    }

    SDL_GL_SwapWindow(ced_sdl_window);

    glPopMatrix();
}

void write_world_into_front_buffer(void){
    ///////
    //   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glPushMatrix();
    //
    //    // TODO: fix it!
    //    // in case of no rotate, in some cases it could get strange
    //    // lines in fisheye view from (0,0,0) to (-inf, -inf,x)
    //    setting.zoom=mm.sf;
    //    glScalef(mm.sf,mm.sf,mm.sf); //zoom
    //
    //    glRotatef(mm.va,1.,0.,0.); //rotate
    //    glRotatef(mm.ha,0.,1.0,0.); //rotate
    //    glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z); //move
    //
    //    if(setting.picking_highlight==true && select_nothing == false){
    //        glColor3f(1,0,0);
    //        glPointSize(10);
    //        glBegin(GL_POINTS);
    //        //cout<< "point: " << pick_point.x << ", " << pick_point.y << ", " << pick_point.z << endl;
    //        glVertex3f(pick_point.x,pick_point.y,pick_point.z);
    //        glEnd();
    //    }
    //    // draw static objects
    //    display_world(); //only axes?
    //
    //    // draw elements (hits + detector)
    //    ced_prepare_objmap();
    //    ced_do_draw_event();
    //
    //
    //    if(showHelp == 1){
    //        printShortcuts();
    //    }
    //
    //
    //    glDisable(GL_LIGHTING);
    //    ced_menu->draw();
    //    popupmenu->draw();
    //    printFPS();
    //
    //    if(setting.light==true){
    //        glEnable(GL_LIGHTING);
    //    }
    //
    //    glutSwapBuffers();
    //
    //    glPopMatrix();
    //
    ///////
    glMatrixMode(GL_PROJECTION);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glRotatef(mm.va,1.,0.,0.);
    //glRotatef(mm.ha,0.,1.0,0.);

    setting.zoom=mm.sf;
    glScalef(mm.sf,mm.sf,mm.sf); //streech the world

    //glScalef(1.19,1.19,1.19);

    //glScalef(1.1,1.1,1.1);


    glRotatef(mm.va,1.,0.,0.); //rotate
    glRotatef(mm.ha,0.,1.0,0.); //rotate
    glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z); //move


      //glMatrixMode(GL_MODELVIEW); //

    // draw static objects

    glMatrixMode(GL_MODELVIEW);

    //glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z);

    display_world();


    //glTranslatef(-mm.mv.x,-mm.mv.y,-mm.mv.z);


   //glTranslatef(0,0,1000);

     //const GLdouble clip_plane[]={0,0,-1,setting.z_cutting};
     //if(setting.z_cutting < 6999){
     //     glEnable(GL_CLIP_PLANE0);
     //}else{
     //     glDisable(GL_CLIP_PLANE0);
     //}
     //glClipPlane(GL_CLIP_PLANE0,clip_plane);


    // draw elements (hits + detector)
    ced_prepare_objmap();

    ced_do_draw_event();


    //cout << "mm.sf: " << mm.sf << "hinterer clipping plane: " << 5000*2.0*mm.sf << std::endl;
    //gluPerspective(60,window_width/window_height,100*2.0*mm.sf,5000*2.0*mm.sf);

    //    std::cout  << "clipping planes: " << 200*2.0*mm.sf << " bis " << 5000*2.0*mm.sf << std::endl;
    //
    //    gluPerspective(60,window_width/window_height,200*2.0*mm.sf,5000*2.0*mm.sf);
    //        glMatrixMode( GL_MODELVIEW );
    //
    //        glLoadIdentity();
    //        gluLookAt  (0,0,2000,    0,0,0,    0,1,0);
    //
    //

    if(setting.light==true){
        glEnable(GL_LIGHTING);
    }


    //printFPS();
}

void reshape(int w,int h){
    // printf("Reshaped: %dx%d\n",w,h);
    window_width=w;
    window_height=h;
    setting.win_w=w;
    setting.win_h=h;



    //if(graphic[3]){
    if(setting.antia){

        //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        //glHint(GL_POLYGON_SMOOTH,GL_FASTEST);

        //glEnable(GL_POINT_SMOOTH);
        //glEnable(GL_LINE_SMOOTH);
        //glEnable(GL_POLYGON_SMOOTH);
        //glShadeModel(GL_SMOOTH);

        //glEnable(GL_BLEND);
        //glEnable (GL_BLEND);
        //glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_LINE_SMOOTH);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    }else{
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
    }

    //if(graphic[2] == 0){
    if(setting.persp == false){

        glViewport(0,0,w,h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-WORLD_SIZE*w/h,WORLD_SIZE*w/h,-WORLD_SIZE,WORLD_SIZE, -15*WORLD_SIZE,15*WORLD_SIZE);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }else{
        glViewport(0,0,w,h);

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        //gluPerspective(60,window_width/window_height,100,500000);
        //double plane1, plane2;
        //plane1=100.0*mm.sf;
        //plane2=50000.0*mm.sf;
        //gluPerspective(60,window_width/window_height,plane1,plane2);
        //gluPerspective(60,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);

        //gluPerspective(45,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);
        glLoadMatrixf(glm::value_ptr(glm::perspective(
            glm::radians((GLfloat)CAMERA_FIELD_OF_VIEW),
            window_width/window_height,
            (GLfloat)CAMERA_MIN_DISTANCE,
            (GLfloat)(CAMERA_MAX_DISTANCE)
        )));

        //gluPerspective(170,window_width/window_height,100.0,50000.0*mm.sf+50000/mm.sf);


        //std::cout  << "clipping planes: " << plane1 << " bis " << plane2<< std::endl;


        glMatrixMode( GL_MODELVIEW );

        glLoadIdentity();

        //glClearDepth(1.0);
        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LEQUAL);
        //glDepthFunc(GL_LESS);




        //glEnable (GL_LINE_SMOOTH);

        //glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);


        //    glShadeModel(GL_SMOOTH);

        //glDepthMask(GL_TRUE);

        // //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        // //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        //glBlendFunc(GL_ONE, GL_ZERO);
        //glEnable(GL_BLEND);

        glMultMatrixf(glm::value_ptr(glm::lookAt(
            glm::vec3(CAMERA_POSITION),
            glm::vec3(0,0,0),
            glm::vec3(0,1,0)
        )));
    }


    //buildMainMenu();
    buildLayerMenus();
}


void idle(void){
    ced_needs_redraw = true;
}


void toggle_layer(unsigned l){
    if(l > CED_MAX_LAYER-1){ return; }

    //    if(ced_visible_layers[l]){
    //        ced_visible_layers[l]=false;
    //    }else{
    //        ced_visible_layers[l]=true;
    //    }


    if(setting.layer[l]){
        setting.layer[l]=false;
    }else{
        setting.layer[l]=true;
    }

}


static void input_data(void *data){
    if(ced_process_input(data)>0){
        ced_needs_redraw = true;
        if( setting.autoshot ) {
          std::cout << " calling screenshot." << std::endl;
          screenshot("/tmp/glced.tga",setting.autoshot_scale);
          reshape((int)window_width, (int)window_height);
        }
    }
}

void toggleHelpWindow(void){ //hauke
    if(showHelp == 1){
        showHelp=0;
    }else{
        showHelp=1;
    }
    ced_needs_redraw = true;
}

static const int kResizeBorder = 6;

static SDL_HitTestResult SDLCALL ced_window_hit_test(SDL_Window *win, const SDL_Point *pt, void *data){
    int w, h;
    SDL_GetWindowSize(win, &w, &h);

    bool left = pt->x < kResizeBorder;
    bool right = pt->x >= w - kResizeBorder;
    bool top = pt->y < kResizeBorder;
    bool bottom = pt->y >= h - kResizeBorder;

    if(top && left) return SDL_HITTEST_RESIZE_TOPLEFT;
    if(top && right) return SDL_HITTEST_RESIZE_TOPRIGHT;
    if(bottom && left) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
    if(bottom && right) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
    if(left) return SDL_HITTEST_RESIZE_LEFT;
    if(right) return SDL_HITTEST_RESIZE_RIGHT;
    if(bottom) return SDL_HITTEST_RESIZE_BOTTOM;
    if(top) return SDL_HITTEST_RESIZE_TOP;

    if(pt->y < CED_TITLE_BAR_HEIGHT) return SDL_HITTEST_DRAGGABLE;

    return SDL_HITTEST_NORMAL;
}

static void mainLoop(SDL_GLContext gl_context) {
    bool running = true;
    ced_needs_redraw = true;

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {

            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED: // Replaces glutReshapeFunc(reshape)
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                reshape(ev.window.data1, ev.window.data2);
                ced_needs_redraw = true;
                break;

            case SDL_EVENT_TEXT_INPUT: // Printable characters (replaces glutKeyboardFunc(keypressed))
                keypressed((unsigned char)ev.text.text[0], 0, 0);
                break;

            case SDL_EVENT_KEY_DOWN: { // Special keys and Ctrl + letter shortcuts (replaces glutSpecialFunc(SpecialKey))
                SDL_Keycode sym = ev.key.key;
                int special = -1;
                switch (sym) {
                    case SDLK_RIGHT: special = KEY_RIGHT; break;
                    case SDLK_LEFT: special = KEY_LEFT; break;
                    case SDLK_UP: special = KEY_UP; break;
                    case SDLK_DOWN: special = KEY_DOWN; break;
                    case SDLK_PAGEUP: special = KEY_PAGE_UP; break;
                    case SDLK_PAGEDOWN: special = KEY_PAGE_DOWN; break;
                    case SDLK_HOME: special = KEY_HOME; break;
                    case SDLK_END: special = KEY_END; break;
                    case SDLK_INSERT: special = KEY_INSERT; break;
                    default: break;
                }
                if (special >= 0) {
                    SpecialKey(special, 0, 0);
                } else if (ev.key.mod & SDL_KMOD_CTRL) {
                    SDL_Scancode sc = ev.key.scancode;
                    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
                        unsigned char key = (unsigned char)(sc - SDL_SCANCODE_A + 1);
                        keypressed(key, 0, 0);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                int btn   = ev.button.button - 1;
                int state = (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? MOUSE_DOWN : MOUSE_UP;
                mouse(btn, state, (int)ev.button.x, (int)ev.button.y);
                break;
            }

            case SDL_EVENT_MOUSE_MOTION:
                if (ev.motion.state != 0) {
                    motion((int)ev.motion.x, (int)ev.motion.y);
                } else {
                    mouse_passive((int)ev.motion.x, (int)ev.motion.y);
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                int dir = (ev.wheel.y > 0) ? 1 : -1;
                mouseWheel(0, dir, 0, 0);
                break;
            }

            default:
                break;
            }
        }

        //  Replace GLUT bult-in socket monitoring with a non-blocking check for incoming client data
        if (socket_fd >= 0 && socket_fn) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(socket_fd, &fds);
            struct timeval tv = {0, 0};
            if (select(
                    socket_fd + 1,
                    &fds,
                    NULL,
                    NULL,
                    &tv
                ) > 0
            )
                socket_fn();
        }

        if (idle_func) {
            idle_func();
            ced_needs_redraw = true;
        }

        if (ced_needs_redraw) {
            display();
            ced_needs_redraw = false;
        } else {
            SDL_Delay(1);
        }
    }

    font_clean();
    SDL_StopTextInput(ced_sdl_window);
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(ced_sdl_window);
    ced_sdl_window = nullptr;
    SDL_Quit();
}

int main(int argc,char *argv[]){
    #ifndef SDL_PLATFORM_APPLE
        setenv("SDL_VIDEODRIVER", "wayland", 0);

        // SDL's Wayland backend initializes xkbcommon directly as part of SDL_Init() to handle
        // keyboard input. The key4hep stack sets XKB_CONFIG_ROOT with a :, which xkbcommon
        // interprets as an empty search path entry and fails to create an XKB context, cascading
        // into SDL Init returning -1. The following code removes this character.
        const char *xkb = getenv("XKB_CONFIG_ROOT");

        if (xkb) {
            std::string s(xkb);

            if (!s.empty() && s.back() == ':') {
                s.pop_back();
                setenv("XKB_CONFIG_ROOT", s.c_str(), 1);
            }
        }
    #endif

    mm_reset=mm;
    WORLD_SIZE = DEFAULT_WORLD_SIZE ;

    SDL_Init(SDL_INIT_VIDEO);
    // SDL's Wayland backend uses EGL which defaults to OpenGL ES. The following code creates a desktop
    // OpenGL compatibility profile context.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    defaultSettings();
    setting.screenshot_sections=1;


    //set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]); //set to default (black)=0;

    //set_bg_color(0.0,0.0,0.0,0.0); //set to default (black)
    //set_bg_color(bgColors[0][0],bgColors[0][1],bgColors[0][2],bgColors[0][3]); //set to default (light blue [0.0, 0.2, 0.4, 0.0])

    //graphic[1]=1; //transp
    //graphic[2]=1; //persp
    //cut_angle=0; //degrees
    //phi_projection=false;
    //projection=false;

    //trans_value=0.8;



    char hex[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int tmp[6];

    int i;
    for(i=1;i<argc ; i++){

      if(!strcmp( argv[i] , "-world_size" ) ) {
        float w_size = atof(  argv[++i] )  ;
        printf( "  setting world size to  %f " , w_size ) ;
        //set_world_size( w_size ) ;
        mm.sf = 205.0/w_size;
      } else if(!strcmp(argv[i], "-bgcolor") && i < argc-1){
        i++;
        if (!strcmp(argv[i],"Black") || !strcmp(argv[i],"black")){
          printf("Set background color to black.\n");
          set_bg_color(0.0,0.0,0.0,0.0); //Black
        } else if (!strcmp(argv[i],"Blue") || !strcmp(argv[i],"blue")){
          printf("Set background color to blue.\n");
          set_bg_color(0.0,0.2,0.4,0.0); //Dark blue
        }else if (!strcmp(argv[i],"White") || !strcmp(argv[i],"white")){
          printf("Set background color to white.\n");
          set_bg_color(1.0,1.0,1.0,0.0); //White
        }else if((strlen(argv[i]) == 8 && argv[i][0] == '0' && toupper(argv[i][1]) == 'X') || strlen(argv[i]) == 6){
          printf("Set background to user defined color.\n");
          int n=0;
          if(strlen(argv[i]) == 8){
              n=2;
          }
          int k;
          for(k=0;k<6;k++){
              int j;
              tmp[k]=999;
              for(j=0;j<16;j++){
                  if(toupper(argv[i][k+n]) == hex[j]){
                      tmp[k]=j;
                  }

              }
              if(tmp[k]==999){
                  printf("Unknown digit '%c'!\nSet background color to default value.\n",argv[i+1][k+n]);
                  break;
              }
              if(k==5){
                  userDefinedBGColor[0] = (tmp[0]*16 + tmp[1])/255.0;
                  userDefinedBGColor[1] = (tmp[2]*16 + tmp[3])/255.0;
                  userDefinedBGColor[2] = (tmp[4]*16 + tmp[5])/255.0;
                  userDefinedBGColor[3] = 0.0;

                  printf("set color to: %f/%f/%f\n",(tmp[0]*16 + tmp[1])/255.0, (tmp[2]*16 + tmp[3])/255.0, (tmp[4]*16 + tmp[5])/255.0);
                  set_bg_color((tmp[0]*16 + tmp[1])/255.0,(tmp[2]*16 + tmp[3])/255.0,(tmp[4]*16 + tmp[5])/255.0,0.0);
              }
          }
        } else {
          printf("Unknown background color.\nPlease choose black/blue/white or a hexadecimal number with 6 digits!\nSet background color to default value.\n");
        }

      } else if(!strcmp( argv[i] , "-h" ) ||
         !strcmp( argv[i] , "--help" )||
         !strcmp( argv[i] , "-?" )
         ) {


      printf( "\n  CED event display server: \n\n"
          "   Usage:  glced [-bgcolor COLOR] [-world_size LENGTH] [-trust TRUSTED_HOST]\n\n"
          "        options:  \n"
          "              COLOR:        Background color (values: black, white, blue or hexadecimal number)\n"
          "              LENGTH:       Visible world-cube size in mm (default: 6000) \n"
          "              TRUSTED_HOST: Ip or name of the host who is allowed to connect to CED\n\n"
          "   Example: \n\n"
          "     ./bin/glced -bgcolor 4C4C66 -world_size 1000. -trust 192.168.11.22 > /tmp/glced.log 2>&1 & \n\n"
          "    "
          "   Change port (before starting glced):"
              "         export CED_PORT=<portnumber>\n\n\n"
          "   To connect Marlin from a remote machine set variables CED_HOST=<this_host> and CED_PORT=<this_CED_PORT> on the machine where Marlin is started from\n\n"
          "   On this machine start glced with option: -trust <host_where_Marlin_is_started_from> to accept the connection from the remote host"
          "\n\n"
          ) ;

        exit(0) ;
      } else if(!strcmp(argv[i], "-trust")){
          i++;
          if(i >= argc){
              printf("wrong syntax!\n");
              exit(0);
          }

          //printf("test: %s %s\n",argv[i], argv[i+1]);
          struct hostent *host = gethostbyname(argv[i]);
          if (host != NULL){
              extern char trusted_hosts[50];
              snprintf(trusted_hosts, 50, "%u.%u.%u.%u",(unsigned char)host->h_addr[0] ,(unsigned char)host->h_addr[1] ,(unsigned char)host->h_addr[2] ,(unsigned char)host->h_addr[3]);
              printf("Trust ip: %s\n", trusted_hosts);
          } else {
              printf("ERROR: Host %s is unknown!\n", argv[i+1]);
          }
      }
    }

    ced_register_elements();

    char *p;
    p = getenv ( "CED_PORT" );
    if(p != NULL){
        printf("Try to use user defined port %s.\n", p);
        tcp_server(atoi(p),input_data);
    }else{
        tcp_server(7286,input_data);
    }


    ced_sdl_window = SDL_CreateWindow(
        "C Event Display (CED)",
        setting.win_w,
        setting.win_h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS
    );

    SDL_SetWindowHitTest(ced_sdl_window, ced_window_hit_test, nullptr); // Implement drag and resize via hit-testing since the window is now borderless

    SDL_GLContext gl_context = SDL_GL_CreateContext(ced_sdl_window); // SDL separates window creation from context creation
    
    if (!gl_context) {
        fprintf(
            stderr,
            "SDL_GL_CreateContext failed: %s\n",
            SDL_GetError()
        );
        SDL_DestroyWindow(ced_sdl_window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1); // vsync control
    SDL_StartTextInput(ced_sdl_window);

    //glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    //glHint(GL_POLYGON_SMOOTH,GL_FASTEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    //glEnable(GL_POLYGON_SMOOTH);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);



    set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]); //set to default (black)
    //glClearColor(BG_COLOR[0],BG_COLOR[1], BG_COLOR[2], BG_COLOR[3]);
    init();
    font_init();   

    buildLayerMenus();
    buildMainMenu();
    popupmenu=new CED_PopUpMenu("");

    //glutAttachMenu(GLUT_RIGHT_BUTTON);
    //for(i=0;i<NUMBER_POPUP_LAYER;i++){ //fill the layer section
    //  updateLayerEntryInPopupMenu(i);
    //}
    //for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){ //fill the layer section
    //  updateLayerEntryDetector(i);
    //}

    //glDisable(GL_BLEND);
    if(setting.light == true){
        setting.light=false;
        selectFromMenu(GRAFIC_LIGHT);
    }


    setting_old[0]=setting;
    setting_old[1]=setting;
    setting_old[2]=setting;
    setting_old[3]=setting;
    setting_old[4]=setting;

    animation_start_time = (int)SDL_GetTicks();

    reshape(setting.win_w, setting.win_h);
    mainLoop(gl_context);

    return 0;
}
