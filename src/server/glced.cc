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
#include "io/settings.h"
#include "io/screenshot.h"
#include "ui/menu.h"

using namespace std;

//*************** global variables ***************************************//
int animation_start_time = 0;
int animate_layer = -1;
int last_selected_layer;
extern CEDsettings setting;
CEDsettings setting_old[5];

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
static void set_bg_color(float one, float two, float three, float four){
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

static void reshape(int w,int h){
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


static void toggle_layer(unsigned l){
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

void update_cut_angle_menu(void){
    return;
    //char str[200];

    //int i;

    //glutSetMenu(subsubMenu2);

    //for(i=0; (unsigned)i < sizeof(available_cutangles)/sizeof(available_cutangles[0]); i++){

    //    if(available_cutangles[i] == setting.cut_angle){
    //        sprintf(str,"[X] %i", available_cutangles[i]);
    //        glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
    //    }else{
    //        sprintf(str,"[  ] %i", available_cutangles[i]);
    //        glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
    //    }
    //}

}

void copySetting(CEDsettings &dest, CEDsettings &source, const char *name){
    if(strcmp(name,"trans")==0){
        for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
            dest.detector_trans[i]=source.detector_trans[i];
        }
    }

    else if(strcmp(name,"cut z")==0){
        for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
            dest.detector_cut_z[i]=source.detector_cut_z[i];
        }
    }

    else if(strcmp(name,"cut angle")==0){
        for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
            dest.detector_cut_angle[i]=source.detector_cut_angle[i];
        }
    }
    else{
        std::cout << "WARNING: unknown settingtype: " << name << std::endl;
    }

}

void selectFromMenu(int id){ //hauke
    int anz;
    static CEDsettings backup_setting;
    //static float z_cutting_backup;
    //static float cut_angle_backup;
    static float mm_ha_backup;
    static float mm_va_backup;
    static int graphic_2_backup;
    //static int fullscreen=false;
    double z_cut;

    if(id != UNDO){
        setting_old[4]=setting_old[3];
        setting_old[3]=setting_old[2];
        setting_old[2]=setting_old[1];
        setting_old[1]=setting_old[0];
        setting_old[0]=setting;
    }

    switch(id){
        case PICK_HIT:

            //cout << "TODO: pick with: " << popupmenu->x_click << " , " << popupmenu->y_click << endl;
            if(!ced_picking(popupmenu->x_click,popupmenu->y_click ,&mm.mv.x,&mm.mv.y,&mm.mv.z)){
               int sel_id = SELECTED_ID;
               if(client_connected){
                    send( socket_fd , &sel_id , sizeof(int) , 0 );
                }
            }
            break;

        case CENTER_HIT:
            if(!ced_get_selected(popupmenu->x_start,popupmenu->y_start,&mm.mv.x,&mm.mv.y,&mm.mv.z)) ced_needs_redraw = true;
            break;

        case BGCOLOR_OPTION1:
            set_bg_color(CED_BGCOLOR_OPTION1_COLORCODE);
            break;

        case BGCOLOR_OPTION2:
            set_bg_color(CED_BGCOLOR_OPTION2_COLORCODE);
            break;

        case BGCOLOR_OPTION3:
            set_bg_color(CED_BGCOLOR_OPTION3_COLORCODE);
            break;

        case BGCOLOR_OPTION4:
            set_bg_color(CED_BGCOLOR_OPTION4_COLORCODE);
            break;

        case BGCOLOR_OPTION5:
            set_bg_color(CED_BGCOLOR_OPTION5_COLORCODE);
            break;

        case BGCOLOR_OPTION6:
            set_bg_color(CED_BGCOLOR_OPTION6_COLORCODE);
            break;

        case BGCOLOR_OPTION7:
            set_bg_color(CED_BGCOLOR_OPTION7_COLORCODE);
            break;

        case BGCOLOR_OPTION8:
            set_bg_color(CED_BGCOLOR_OPTION8_COLORCODE);
            break;

        case BGCOLOR_OPTION9:
            set_bg_color(CED_BGCOLOR_OPTION9_COLORCODE);
            break;

        case BGCOLOR_OPTION10:
            set_bg_color(CED_BGCOLOR_OPTION10_COLORCODE);
            break;

        case BGCOLOR_OPTION11:
            set_bg_color(CED_BGCOLOR_OPTION11_COLORCODE);
            break;

        case BGCOLOR_OPTION12:
            set_bg_color(CED_BGCOLOR_OPTION12_COLORCODE);
            break;

        case BGCOLOR_OPTION13:
            set_bg_color(CED_BGCOLOR_OPTION13_COLORCODE);
            break;

        case BGCOLOR_OPTION14:
            set_bg_color(CED_BGCOLOR_OPTION14_COLORCODE);
            break;

        case BGCOLOR_OPTION15:
            set_bg_color(CED_BGCOLOR_OPTION15_COLORCODE);
            break;






    //        case BGCOLOR_GAINSBORO:
    //            set_bg_color(0.862745,0.862745,0.862745,0);
    //            //set_bg_color(0.862745,0.862745,0.862745,0);
    //            break;
    //
    //        case BGCOLOR_LIGHTGREY:
    //            set_bg_color(0.827451,0.827451,0.827451,0);
    //            break;
    //
    //        case BGCOLOR_DARKGRAY:
    //            set_bg_color(0.662745,0.662745,0.662745,0);
    //            break;
    //
    //        case BGCOLOR_GRAY:
    //            set_bg_color(0.501961,0.501961,0.501961,0);
    //            break;
    //
    //        case BGCOLOR_SILVER:
    //            set_bg_color(0.7529,0.7529,0.7529,0);
    //            break;
    //
    //        case BGCOLOR_DIMGRAY:
    //            set_bg_color(0.4118,0.4118,0.4118,0);
    //            break;
    //
    //        case BGCOLOR_LIGHTSTEELBLUE:
    //            set_bg_color(0.6902,0.7686 ,0.8706,0);
    //            break;
    //
    //        case BGCOLOR_STEELBLUE:
    //            set_bg_color(0.2745,0.5098,0.70588,0);
    //            break;
    //
    //        case BGCOLOR_SEAGREEN:
    //            set_bg_color(0.18039,0.54509,0.34117,0);
    //            break;
    //
    //        case BGCOLOR_ORANGE:
    //            set_bg_color(1,0.647,0,0);
    //            break;
    //
    //        case BGCOLOR_YELLOW:
    //            set_bg_color(1,1,0,0);
    //            break;
    //
    //        case BGCOLOR_VIOLET:
    //            set_bg_color(0.9333,0.5098,0.9333,0);
    //            break;
    //
    //        case BGCOLOR_BLACK:
    //            set_bg_color(0,0,0,0);
    //            break;
    //
    //        case BGCOLOR_BLUE:
    //            set_bg_color(0,0.2,0.4,0);
    //            break;
    //
    //        case BGCOLOR_WHITE:
    //            set_bg_color(1,1,1,0);
    //            break;

        case BGCOLOR_USER:
            set_bg_color(userDefinedBGColor[0],userDefinedBGColor[1], userDefinedBGColor[2], userDefinedBGColor[3]);
            [[fallthrough]];

        case VIEW_RESET:
            setting.phi_projection = false; // no phi projection
            setting.z_projection=false; // no phi projection;
            mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            fisheye_alpha=0;
            setting.fixed_view=false;
            //update_cut_angle_menu();
            set_world_size(DEFAULT_WORLD_SIZE );
            break;

        case CED_RESET:
            //if(graphic[2] == 0){selectFromMenu(GRAFIC_PERSP); }
            if((setting.trans == true && setting.persp == false) || (setting.trans == false && setting.persp == true)){
                selectFromMenu(GRAFIC_PERSP); //switch persp on in new view, switch persp off in classic view
            }
            //setting.z_cutting=7000; //no z cutting
            //setting.cut_angle=0;    // no detector cutting
            for(int i = 0; i<NUMBER_DETECTOR_LAYER;i++){
                setting.detector_trans[i]=0.8;
                setting.detector_cut_angle[i]=0;
                setting.detector_cut_z[i]=7000;
            }

            for(int i = 0; i<CED_MAX_LAYER;i++){
                setting.layer[i]=true;
            }
            setting.phi_projection = false; // no phi projection
            setting.z_projection=false; // no phi projection;
            mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            fisheye_alpha=0;
            setting.fixed_view=false;
            //update_cut_angle_menu();
            set_world_size(DEFAULT_WORLD_SIZE );
            //std::cout << "DEFAULT_WORLD_SIZE "  << DEFAULT_WORLD_SIZE << "zoom: " << mm.sf << std::endl;
            setting.light=false;

            setting.show_axes=true;
            break;


        case VIEW_FISHEYE:
            if(fisheye_alpha==0.0){
                mm.sf *= FISHEYE_ZOOM; //zoom in to hold the same detector size
                fisheye_alpha = FISHEYE_ALPHA;
                FISHEYE_WORLD_SIZE = WORLD_SIZE/(WORLD_SIZE*fisheye_alpha); //<-- new
                set_world_size(WORLD_SIZE); // <-- old
            }
            else{
                mm.sf *= 1.0/FISHEYE_ZOOM; //zoom out for the same look
                fisheye_alpha = 0.0;
                set_world_size(FISHEYE_WORLD_SIZE); //<-- old
            }
            break;

        case VIEW_FRONT:
            //mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;

            if(setting.fixed_view){ break;}

            mm.ha=180.;
            mm.va=0.;
            break;

        case VIEW_SIDE:
            //mm=mm_reset;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            if(setting.fixed_view){ break;}

                mm.ha=90.;
                mm.va=0.;

            break;

        case TOGGLE_PHI_PROJECTION:
            if(setting.phi_projection){ //turn projection off
                setting.phi_projection=false;
                //setting.z_cutting=z_cutting_backup;
                //copySetting(setting, backup_setting, "cut z");
                //setting.detector_cut_z=backup_setting.detector_cut_z;
                //setting.cut_angle=cut_angle_backup;


                copySetting(setting, backup_setting, "cut angle");
                //setting.detector_cut_angle=backup_setting.detector_cut_angle;
                //if(graphic_2_backup != graphic[2]){selectFromMenu(GRAFIC_PERSP); } //restore persp setting
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting

                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;

            }else{ //turn projection on
                if(setting.z_projection){
                    selectFromMenu(TOGGLE_Z_PROJECTION);
                }

                //z_cutting_backup=setting.z_cutting;
                //cut_angle_backup=setting.cut_angle;

                copySetting(backup_setting, setting, "cut angle");
                //copySetting(backup_setting, setting, "cut z");
                //backup_setting.detector_cut_z=setting.detector_cut_z;
                //backup_setting.detector_cut_angle=setting.detector_cut_angle;

                setting.phi_projection=true;

                //graphic_2_backup=graphic[2];
                graphic_2_backup=setting.persp;

                //if(graphic[2]==1){selectFromMenu(GRAFIC_PERSP); }
                if(setting.persp==1){selectFromMenu(GRAFIC_PERSP); }


                for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
                    setting.detector_cut_angle[i]=180;
                //    setting.detector_cut_z[i]=7000;
                }
                //setting.cut_angle=180;
                //setting.z_cutting=7000;
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;
                mm.ha=90.;
                mm.va=0.;

                setting.fixed_view=true;
            }

            update_cut_angle_menu();
            break;

        case TOGGLE_Z_PROJECTION:
            if(setting.z_projection){ //turn projection off

                setting.z_projection=false;
                //z_cutting=7000;
                //selectFromMenu(GRAFIC_PERSP);
                //setting.z_cutting=z_cutting_backup;

                copySetting(setting, backup_setting, "cut z");
                //copySetting(setting, backup_setting, "cut angle");
                //setting.detector_cut_z = backup_setting.detector_cut_z;
                //setting.cut_angle=cut_angle_backup;
                //setting.detector_cut_angle = backup_setting.detector_cut_angle;
                //if(graphic[2]==0){selectFromMenu(GRAFIC_PERSP); }
                //if(graphic_2_backup != graphic[2]){selectFromMenu(GRAFIC_PERSP); } //restore persp setting
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting


                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;
            }else{ //turn projection on

                if(setting.phi_projection){selectFromMenu(TOGGLE_PHI_PROJECTION);}

                //z_cutting_backup=setting.z_cutting;

                copySetting(backup_setting, setting, "cut z");
                //cout << "toggle z projection!:" << endl;
                //for(int i = 0; i<NUMBER_DETECTOR_LAYER;i++){
                //    cout << backup_setting.detector_cut_z[i] << endl;
                //}



                //copySetting(backup_setting, setting, "cut angle");



                //backup_setting.detector_cut_z=setting.detector_cut_z;
                //cut_angle_backup=setting.cut_angle;
                //backup_setting.detector_cut_angle=setting.detector_cut_angle;

                setting.z_projection=true;

                for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
                    //setting.cut_angle=0;
                    //setting.z_cutting=-10;
                    setting.detector_cut_z[i]=-10;
                }

                //graphic_2_backup=graphic[2];
                graphic_2_backup=setting.persp;

                //if(graphic[2]==1){selectFromMenu(GRAFIC_PERSP); }
                if(setting.persp==true){selectFromMenu(GRAFIC_PERSP); }


               //side view
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;

                mm.ha=180.;
                mm.va=0.;

                setting.fixed_view=true;
            }

            update_cut_angle_menu();
            break;

        case FONT0:
            setting.font=FONT_S;
            //buildMainMenu();
            break;

        case FONT1:
            setting.font=FONT_M;
            //buildMainMenu();
            break;

        case FONT2:
            setting.font=FONT_L;
            //buildMainMenu();
            break;


        case UNDO:
            setting=setting_old[0];
            setting_old[0]=setting_old[1];
            setting_old[1]=setting_old[2];
            setting_old[2]=setting_old[3];
            setting_old[3]=setting_old[4];
            setting_old[4]=setting_old[4];
            break;


        case VIEW_ZOOM_IN:
            mm.sf += mm.sf*50.0/window_height;
            //cout << "mm.sf: " << mm.sf << endl;
            //reshape((int)window_width, (int)window_height);

            //if(mm.sf>50){ mm.sf=50; }
            break;

        case VIEW_ZOOM_OUT:
            mm.sf -= mm.sf*50.0/window_height;
            //cout << "mm.sf: " << mm.sf << endl;

            //if(mm.sf > 0.05){
            //    mm.sf -= 0.05;
            //}else{
            //    mm.sf=0.00001;
            //}
            //reshape((int)window_width, (int)window_height);

            //if(mm.sf<0.01){ mm.sf=0.01; }
            break;

        case VIEW_CENTER:
            //ced_get_selected(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z);
            break;

        case LAYER_ALL:
            anz=0;
            //for(i=0;i<NUMBER_POPUP_LAYER;i++){ //try to turn all layers on

            for(int i=0;i<NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   //sprintf(string,"[X] Layer %s%i [%c]: %s", (i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);
                   toggle_layer(i);
                   //updateLayerEntryInPopupMenu(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                //for(i=0;i<NUMBER_POPUP_LAYER;i++){

                for(int i=0;i<NUMBER_DATA_LAYER;i++){
                   //sprintf(string,"[   ] Layer %s%i [%c]: %s",(i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);
                   toggle_layer(i);
                   //updateLayerEntryInPopupMenu(id);
                }
            }
            break;

        case DETECTOR_ALL:
            anz=0;
            for(int i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   //sprintf(string,"[X] Layer %s%i [%c]: %s", (i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);
                   toggle_layer(i);
                   //updateLayerEntryDetector(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                for(int i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
                   //sprintf(string,"[   ] Layer %s%i [%c]: %s",(i < 10)?"0":"" ,i, layer_keys[i], layerDescription[i]);
                   //glutChangeToMenuEntry(i+2,string, LAYER_0+i);
                   toggle_layer(i);
                   //updateLayerEntryDetector(id);
                }
            }
            break;


        case DETECTOR1:
        case DETECTOR2:
        case DETECTOR3:
        case DETECTOR4:
        case DETECTOR5:
        case DETECTOR6:
        case DETECTOR7:
        case DETECTOR8:
        case DETECTOR9:
        case DETECTOR10:
        case DETECTOR11:
        case DETECTOR12:
        case DETECTOR13:
        case DETECTOR14:
        case DETECTOR15:
        case DETECTOR16:
        case DETECTOR17:
        case DETECTOR18:
        case DETECTOR19:
        case DETECTOR20:
        case DETECTOR21:
        case DETECTOR22:
        case DETECTOR23:
        case DETECTOR24:
        case DETECTOR25:
        case DETECTOR26:
        case DETECTOR27:
        case DETECTOR28:
        case DETECTOR29:
        case DETECTOR30:
        case DETECTOR31:
        case DETECTOR32:
        case DETECTOR33:
        case DETECTOR34:
        case DETECTOR35:
        case DETECTOR36:
        case DETECTOR37:
        case DETECTOR38:
        case DETECTOR39:
        case DETECTOR40:

            toggle_layer(id-DETECTOR1+NUMBER_DATA_LAYER);
            //std::cout << "toogle layer " << id-DETECTOR1 + NUMBER_DATA_LAYER<< std::endl;
            //updateLayerEntryDetector(id-DETECTOR1+NUMBER_DATA_LAYER);

            break;



        case LAYER_0:
        case LAYER_1:
        case LAYER_2:
        case LAYER_3:
        case LAYER_4:
        case LAYER_5:
        case LAYER_6:
        case LAYER_7:
        case LAYER_8:
        case LAYER_9:
        case LAYER_10:
        case LAYER_11:
        case LAYER_12:
        case LAYER_13:
        case LAYER_14:
        case LAYER_15:
        case LAYER_16:
        case LAYER_17:
        case LAYER_18:
        case LAYER_19:
        case LAYER_20:
        case LAYER_21:
        case LAYER_22:
        case LAYER_23:
        case LAYER_24:
            toggle_layer(id-LAYER_0);
            //std::cout << "toogle layer " << id-LAYER_0 << std::endl;
            //updateLayerEntryInPopupMenu(id-LAYER_0);
            //buildLayerMenus();
            //buildMainMenu();

            break;

        case CUT_ANGLE0  :
        case CUT_ANGLE30 :
        case CUT_ANGLE90 :
        case CUT_ANGLE135:
        case CUT_ANGLE180:
        case CUT_ANGLE200:
        case CUT_ANGLE220:
        case CUT_ANGLE240:
        case CUT_ANGLE260:
        case CUT_ANGLE270:
        case CUT_ANGLE280:
        case CUT_ANGLE290:
        case CUT_ANGLE310:
        case CUT_ANGLE330:
        case CUT_ANGLE340:
        case CUT_ANGLE350:
        case CUT_ANGLE45 :
        case CUT_ANGLE100:
        case CUT_ANGLE120:
        case CUT_ANGLE150:
        case CUT_ANGLE170:
        case CUT_ANGLE190:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_angle[i]=id-CUT_ANGLE0;
            break;




        case LAYER_CUT_ANGLE0  :
        case LAYER_CUT_ANGLE30 :
        case LAYER_CUT_ANGLE90 :
        case LAYER_CUT_ANGLE135:
        case LAYER_CUT_ANGLE180:
        case LAYER_CUT_ANGLE200:
        case LAYER_CUT_ANGLE220:
        case LAYER_CUT_ANGLE240:
        case LAYER_CUT_ANGLE260:
        case LAYER_CUT_ANGLE270:
        case LAYER_CUT_ANGLE280:
        case LAYER_CUT_ANGLE290:
        case LAYER_CUT_ANGLE310:
        case LAYER_CUT_ANGLE330:
        case LAYER_CUT_ANGLE340:
        case LAYER_CUT_ANGLE350:
        case LAYER_CUT_ANGLE45 :
        case LAYER_CUT_ANGLE100:
        case LAYER_CUT_ANGLE120:
        case LAYER_CUT_ANGLE150:
        case LAYER_CUT_ANGLE170:
        case LAYER_CUT_ANGLE190:
            setting.detector_cut_angle[last_selected_layer- NUMBER_DATA_LAYER]=id-LAYER_CUT_ANGLE0;
            break;





        case CUT_Z_7000:z_cut=7000;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_M6000:z_cut=-6000;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_M4000:z_cut=-4000;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_M2000:z_cut=-2000;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_0000 :z_cut=0 ;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_2000 :z_cut=2000 ;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_4000 :z_cut=4000 ;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case CUT_Z_6000 :z_cut=6000 ;
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
               setting.detector_cut_z[i]=z_cut;
            break;

        case LAYER_CUT_Z_7000:z_cut=7000;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_M4000:z_cut=-4000;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_M2000:z_cut=-2000;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_0000 :z_cut=0 ;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_2000 :z_cut=2000 ;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_4000 :z_cut=4000 ;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;

        case LAYER_CUT_Z_6000 :z_cut=6000 ;
            setting.detector_cut_z[last_selected_layer- NUMBER_DATA_LAYER]=z_cut;
            break;



        case TRANS0:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0;
            //setting.trans_value=0;
            break;
        case TRANS40:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.4;
            //setting.trans_value=0.4;
            break;
        case TRANS60:
             for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.6;
            //setting.trans_value=0.6;
            break;
        case TRANS70:
             for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.7;
            //setting.trans_value=0.7;
            break;
        case TRANS80:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.8;
            //setting.trans_value=0.8;
            break;
        case TRANS90:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.9;
            //setting.trans_value=0.9;
            break;
        case TRANS95:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=0.95;
            //setting.trans_value=0.95;
            break;
        case TRANS100:
            for(int i=0;i<NUMBER_DETECTOR_LAYER;i++)
                setting.detector_trans[i]=1.0;
            //setting.trans_value=1.0;
            break;

        case LAYER_TRANS0:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0;
            break;
        case LAYER_TRANS40:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.4;
            break;
        case LAYER_TRANS60:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.6;
            break;
        case LAYER_TRANS70:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.7;
            break;
        case LAYER_TRANS80:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.8;
            break;
        case LAYER_TRANS90:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.9;
            break;
        case LAYER_TRANS95:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=0.95;
            break;
        case LAYER_TRANS100:
            setting.detector_trans[last_selected_layer- NUMBER_DATA_LAYER]=1.0;
            break;

    //        case FULLSCREEN:
    //////            glutDestroyWindow(mainWindow);;
    //////            glutGameModeString("1280x1024:32@60");
    //////            glutEnterGameMode();
    ////            if(fullscreen == false){
    ////                glutFullScreen();
    ////                fullscreen = true;
    ////            }else{
    ////                fullscreen = false;
    ////                reshape(setting.win_w, setting.win_h);
    ////            }
    //
        case AXES:
            if(setting.show_axes){
                setting.show_axes= false;
            }else{
                setting.show_axes= true;
            }
            break;

        case FPS:
            //cout << "call fps" << endl;
            if(setting.fps){
                idle_func = nullptr;
                setting.fps=false;
            }else{
                idle_func = idle;
                setting.fps=true;
            }
            break;

        case AUTOSHOT:
           if(setting.autoshot) {
             setting.autoshot=false;
           }
           else {
             setting.autoshot=true;
           }
           break;

        case GRAFIC_HIGH:
            setting.light=true;
            setting.trans=false;
            setting.persp=false;
            selectFromMenu(GRAFIC_TRANS);
            selectFromMenu(GRAFIC_LIGHT);
            selectFromMenu(GRAFIC_PERSP);
            break;

        case GRAFIC_LOW:
            setting.light=true;
            setting.trans=true;
            setting.persp=true;
            selectFromMenu(GRAFIC_TRANS);
            selectFromMenu(GRAFIC_LIGHT);
            selectFromMenu(GRAFIC_PERSP);
            break;

        case GRAFIC_TRANS:
    /*
            if(graphic[1] == 1){
                //printf("Transparency  is now off\n");
                graphic[1] = 0;
            }else{
                //printf("Transparency  is now on\n");
                graphic[1] = 1;
            }

    */

            if(setting.trans == true){
                //printf("Transparency  is now off\n");
                setting.trans = false;
            }else{
                //printf("Transparency  is now on\n");
                setting.trans = true;
            }

            break;

        case GRAFIC_LIGHT:
            //if(graphic[0] == 1){
            if(setting.light == true){

                //printf("Light  is now on\n");
                //graphic[0] = 0;
                setting.light=false;
                glDisable(GL_LIGHTING);
            }else{

                 //printf("Light is now on\n");
                 //graphic[0] = 1;
                 setting.light = true;

                // break; //do nothing...

                 //TODO: CHANGE IT
                 GLfloat light0_spec[] = {1, 1, 1, 0.5};
                 GLfloat light0_ambi[] = {1, 1, 1, 0.5};
                 GLfloat light0_diff[] = {1, 1, 1, 0.5};
    //      mm.ha=mm.ha_start+(x-mouse_x)*180./window_width;
    //      mm.va=mm.va_start+(y-mouse_y)*180./window_height;


                 GLfloat light0_pos[] = {20000, 20000, 20000};

            glBegin(GL_QUADS);
            glVertex3d(2000,2000,20000);
            glVertex3d(2500,2000,20000);
            glVertex3d(2000,2500,20000);
            glVertex3d(2000,2000,20500);
            glEnd();

                 //GLfloat light0_dir[] = {-1, -1, 0};

                 //GLfloat angle[] = {30};
                 //GLfloat light0_ambi[]= {0.5, 0.5, 0.5, 0.5};

    /////////////////



    ///////////////////


                 glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);
                 glLightfv(GL_LIGHT1, GL_DIFFUSE, light0_ambi);
                 glLightfv(GL_LIGHT2, GL_AMBIENT, light0_diff);

                 //glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, angle);
                 //glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, angle);
                 //glLightfv(GL_LIGHT2, GL_SPOT_CUTOFF, angle);



                 glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
                 glLightfv(GL_LIGHT1, GL_POSITION, light0_pos);
                 glLightfv(GL_LIGHT2, GL_POSITION, light0_pos);

                 //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_dir);
                 //glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light0_dir);
                 //glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light0_dir);


                 ////glClearColor (0.0, 0.0, 0.0, 0.0);
                 //glShadeModel (GL_SMOOTH);



                 //glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
                 //glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

                 glColorMaterial ( GL_FRONT_AND_BACK, GL_EMISSION ) ;
                 glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE) ;
                 glEnable (GL_COLOR_MATERIAL) ;


                 glEnable(GL_NORMALIZE);

                 glEnable(GL_LIGHTING);
                 //glEnable(GL_LIGHT0);
                 glEnable(GL_LIGHT0);

                 glEnable(GL_DEPTH_TEST);

                 glMatrixMode(GL_MODELVIEW);
            }
            break;

        case GRAFIC_ALIAS:
            //if(graphic[3] == 1){
            if(setting.antia == true){
                //printf("Anti aliasing is off\n");
                //graphic[3] = 0;
                setting.antia = false;
                reshape((int)window_width, (int)window_height);
            }else{
                //printf("Anti aliasing is on\n");
                //graphic[3] = 1;
                setting.antia=true;
                reshape((int)window_width, (int)window_height);
            }
            break;

        case PICKING_MARKER:
            if(setting.picking_highlight==true){
                setting.picking_highlight=false;
            }else{
                setting.picking_highlight=true;
            }
            break;

        case TOGGLE_DETECTOR_PICKING:
            setting.detector_picking = abs(setting.detector_picking-1);
            break;

        case GRAFIC_FOG:
                glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
                //GLfloat fogcolor[4]={setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],1.0};
                GLfloat fogcolor[4];
                fogcolor[0]=setting.bgcolor[0];
                fogcolor[1]=setting.bgcolor[1];
                fogcolor[2]=setting.bgcolor[2];
                fogcolor[3]=0.5;


                glFogfv(GL_FOG_COLOR,fogcolor);
                glFogf(GL_FOG_DENSITY,0.5);
                //glFogi(GL_FOG_MODE,GL_EXP);

                glFogi(GL_FOG_MODE,GL_LINEAR);
                glFogf(GL_FOG_START,500.0);
                glFogf(GL_FOG_END,3000.0);
                glHint(GL_FOG_HINT, GL_FASTEST);
                glEnable(GL_FOG);
                break;


        case GRAFIC_PERSP:
            if(setting.persp == true){
                //printf("Perspective is now flat\n");
                setting.persp = false;

                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }else{
                //printf("Perspective is now 3d\n");
                setting.persp = true;
                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }
            break;
        case HELP:
            toggleHelpWindow();
            break;
        case SAVE_IMAGE1:
            setting.autoshot_scale=1;
            screenshot("/tmp/glced.tga",1);
            reshape((int)window_width, (int)window_height);
            break;
        case SAVE_IMAGE4:
            setting.autoshot_scale=4;
            screenshot("/tmp/glced.tga",4);
            reshape((int)window_width, (int)window_height);
            break;
        case SAVE_IMAGE10:
            setting.autoshot_scale=10;
            screenshot("/tmp/glced.tga",10);
            reshape((int)window_width, (int)window_height);
            break;
        case SAVE_IMAGE20:
            setting.autoshot_scale=20;
            screenshot("/tmp/glced.tga",20);
            reshape((int)window_width, (int)window_height);
            break;
        case SAVE_IMAGE100:
            setting.autoshot_scale=100;
            screenshot("/tmp/glced.tga",100);
            reshape((int)window_width, (int)window_height);
            break;

        case AUTOSHOT_IMAGE1:
           setting.autoshot_scale=1;
           break;
        case AUTOSHOT_IMAGE4:
           setting.autoshot_scale=4;
           break;
        case AUTOSHOT_IMAGE10:
           setting.autoshot_scale=10;
           break;
        case AUTOSHOT_IMAGE20:
           setting.autoshot_scale=20;
           break;
        case AUTOSHOT_IMAGE100:
           setting.autoshot_scale=100;
           break;

    }

    //reshape((int)window_width, (int)window_height);


    if(id != 0){
    //buildMainMenu();
        buildLayerMenus();
    }

    ced_needs_redraw = true;
    //printf("bgcolor = %f %f %f %f\n",setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[2]);



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
