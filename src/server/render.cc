/* OpenGL rendering for CED's event display server.
 * Split out of glced.cc: window/GL-state init, the per-frame draw,
 * and viewport/projection setup. */

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SDL3/SDL.h>

#include <ced.h>
#include <ced_cli.h>
#include <ced_config.h>
#include <ced_menu.h>

#include "third_party/fg_geometry.h"
#include "ui/input.h"
#include "ui/menu.h"
#include "ui/overlay.h"
#include "ui/selection.h"

#include "render.h"

extern float BG_COLOR[4];
extern GLfloat axe[][3];
extern int showHelp;

extern CED_Menu *ced_menu;
extern CED_PopUpMenu *popupmenu;
extern SDL_Window *ced_sdl_window;

extern float WORLD_SIZE;

extern Point pick_point;
extern Point pre_pick_point;
extern int selected_layer;
extern bool select_nothing;

void init(void){
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

void display(void){
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
