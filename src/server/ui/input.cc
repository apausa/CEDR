#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <sys/socket.h>
#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>

#include <ced.h>
#include <settings.h>

#include "actions.h"
#include "input.h"
#include "selection.h"
#include "utils/helpers.h"

using namespace std;

// Owned by glced.cc.
extern long int doubleClickTime;
extern int last_selected_layer;
extern int selected_layer;
extern Point pick_point;
extern Point pre_pick_point;
extern bool select_nothing;
extern bool ced_needs_redraw;
extern int socket_fd;
extern bool client_connected;
extern float userDefinedBGColor[];
extern CEDsettings setting;

CameraState mm = {
    30.,
    150.,
    0.1, //hauke decrease zoom, //SJA:FIXED set redraw scale a lot smaller
    { 0., 0., 0. },
    0.,
    0.,
    1.,
    { 0., 0., 0. },
};
CameraState mm_reset;

static enum {
    NO_MOVE,
    TURN_XY,
    ZOOM,
    ORIGIN
} move_mode;
static GLfloat mouse_x=0.;
static GLfloat mouse_y=0.;

typedef GLfloat color_t[4];

static color_t bgColors[] = {
  { 0.0, 0.2, 0.4, 0.0 }, //light blue
  { 0.0, 0.0, 0.0, 0.0 }, //black
  { 0.2, 0.2, 0.2, 0.0 }, //gray shades
  { 0.4, 0.4, 0.4, 0.0 },
  { 0.6, 0.6, 0.6, 0.0 },
  { 0.8, 0.8, 0.8, 0.0 },
  { 1.0, 1.0, 1.0, 0.0 }  //white
};
static unsigned int iBGcolor = 0;

void mouseWheel(int, int dir, int, int ){ //hauke
    if(dir > 0){
        selectFromMenu(VIEW_ZOOM_IN);
    }else{
        selectFromMenu(VIEW_ZOOM_OUT);
    }
}

void mouse(int btn,int state,int x,int y){
    //hauke
    struct timeval tv;

    if(state!=MOUSE_DOWN){
        move_mode=NO_MOVE;
        return;
    }
    mouse_x=x;
    mouse_y=y;
    mm.ha_start=mm.ha;
    mm.va_start=mm.va;
    mm.sf_start=mm.sf;
    mm.mv_start=mm.mv;

    //double angle;
    switch(btn){
    case MOUSE_LEFT:
        //reshape((int)window_width, (int)window_height);
        ced_needs_redraw = true;



        //hauke
        gettimeofday(&tv, 0);
        //FIX IT: get the system double click time
        if( (tv.tv_sec*1000000+tv.tv_usec-doubleClickTime) < 300000 && (tv.tv_sec*1000000+tv.tv_usec-doubleClickTime) > 5){ //1000000=1sec

            last_selected_layer=-1;
            //printf("Double Click %f\n", tv.tv_sec*1000000+tv.tv_usec-doubleClickTime);
            if(!ced_picking(x,y,&mm.mv.x,&mm.mv.y,&mm.mv.z)){


                GLfloat p_x, p_y, p_z;
                int id, layer, type;
                if(!find_selected_object(x,y,&p_x,&p_y,&p_z, &id, &layer, &type)){ //if ==1 found hit, else clicked on background
                    pick_point.x=p_x;
                    pick_point.y=p_y;
                    pick_point.z=p_z;

                    select_nothing=false;

                    if(type == 1){ //detector
                        selected_layer=layer;
                        last_selected_layer=layer;
                        select_nothing=true;
                    }else if(type == 0){ //data
                        selected_layer=-1;
                        pre_pick_point.x=p_x;
                        pre_pick_point.y=p_y;
                        pre_pick_point.z=p_z;
                    }


                    if(setting.detector_picking==false){
                        selected_layer=-1;
                    }
                }


               id = SELECTED_ID;
               if(client_connected){
                    send( socket_fd , &id , sizeof(int) , 0 );
                }
            }else{
                select_nothing=true;
                selected_layer=-1;
            }


        }else{
            //printf("Single Click\n");
            if(setting.fixed_view == 0){ //dont rotate the view when in side or front projection
                move_mode=TURN_XY;
            }
        }
        doubleClickTime=tv.tv_sec*1000000+tv.tv_usec;
        return;
        case MOUSE_RIGHT:
          ced_needs_redraw = true;
          if(ZOOM_RIGHT_CLICK == false){
            return;
          }
          move_mode=ZOOM;
          return;
        case MOUSE_MIDDLE:
          //cout << "middle button clicked" << endl;
          //#ifdef __APPLE__
          //    move_mode=ZOOM;
          //#else
          //    move_mode=ORIGIN;
	      //#endif
          move_mode=ORIGIN;
          return;
        default:
          break;
    }
}

#define SELECT_FROM_MENU(key, action)                                          \
  case key:                                                                    \
    selectFromMenu(action);                                                    \
    break


void keypressed(unsigned char key, int x, int y) {
  // SM-H: TODO: socket list for communicating with client
  // struct __glutSocketList *sock;
  // if(key==0x1A ){ //ctrl+z

  // if(key=='u' ){ //ctrl+z

  switch (key) {
    SELECT_FROM_MENU('r', VIEW_RESET);
    SELECT_FROM_MENU('R', CED_RESET);
    SELECT_FROM_MENU('f', VIEW_FRONT);
    SELECT_FROM_MENU('F', TOGGLE_Z_PROJECTION);
    SELECT_FROM_MENU('s', VIEW_SIDE);
    SELECT_FROM_MENU('S', TOGGLE_PHI_PROJECTION);
    SELECT_FROM_MENU('+', VIEW_ZOOM_IN);
    SELECT_FROM_MENU('-', VIEW_ZOOM_OUT);

    SELECT_FROM_MENU(26, UNDO);
    SELECT_FROM_MENU('x', UNDO);
  case 27: // esc
    exit(0);
  case 'c':
  case 'C':
    // selectFromMenu(VIEW_CENTER);
    if (!ced_get_selected(x, y, &mm.mv.x, &mm.mv.y, &mm.mv.z)) {
      ced_needs_redraw = true;
    }
    break;

    SELECT_FROM_MENU('`', LAYER_ALL);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_00, LAYER_0);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_01, LAYER_1);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_02, LAYER_2);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_03, LAYER_3);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_04, LAYER_4);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_05, LAYER_5);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_06, LAYER_6);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_07, LAYER_7);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_08, LAYER_8);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_09, LAYER_9);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_10, LAYER_10);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_11, LAYER_11);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_12, LAYER_12);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_13, LAYER_13);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_14, LAYER_14);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_15, LAYER_15);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_16, LAYER_16);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_17, LAYER_17);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_18, LAYER_18);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_19, LAYER_19);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_20, LAYER_20);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_21, LAYER_21);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_22, LAYER_22);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_23, LAYER_23);
    SELECT_FROM_MENU(DATALAYER_SHORTKEY_24, LAYER_24);

    SELECT_FROM_MENU('~', DETECTOR_ALL);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_00, DETECTOR1);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_01, DETECTOR2);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_02, DETECTOR3);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_03, DETECTOR4);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_04, DETECTOR5);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_05, DETECTOR6);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_06, DETECTOR7);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_07, DETECTOR8);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_08, DETECTOR9);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_09, DETECTOR10);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_10, DETECTOR11);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_11, DETECTOR12);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_12, DETECTOR13);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_13, DETECTOR14);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_14, DETECTOR15);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_15, DETECTOR16);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_16, DETECTOR17);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_17, DETECTOR18);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_18, DETECTOR19);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_19, DETECTOR20);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_20, DETECTOR21);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_21, DETECTOR22);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_22, DETECTOR23);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_23, DETECTOR24);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_24, DETECTOR25);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_25, DETECTOR26);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_26, DETECTOR27);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_27, DETECTOR28);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_28, DETECTOR29);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_29, DETECTOR30);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_30, DETECTOR31);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_31, DETECTOR32);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_32, DETECTOR33);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_33, DETECTOR34);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_34, DETECTOR35);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_35, DETECTOR36);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_36, DETECTOR37);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_37, DETECTOR38);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_38, DETECTOR39);
    SELECT_FROM_MENU(DETECTORLAYER_SHORTKEY_39, DETECTOR40);

  case 'z':
    if (last_selected_layer > 0) {
      if (setting.detector_cut_z[last_selected_layer - NUMBER_DATA_LAYER] <
          7000) {
        setting.detector_cut_z[last_selected_layer - NUMBER_DATA_LAYER] += 100;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_cut_z[0] < 7000) {
          setting.detector_cut_z[i] += 100;
        }
      }
    }
    ced_needs_redraw = true;
    break;

  case 'Z':
    if (last_selected_layer > 0) {
      if (setting.detector_cut_z[last_selected_layer - NUMBER_DATA_LAYER] >
          -7000) {
        setting.detector_cut_z[last_selected_layer - NUMBER_DATA_LAYER] -= 100;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_cut_z[i] > -7000) {
          setting.detector_cut_z[i] -= 100;
        }
      }
    }
    ced_needs_redraw = true;
    break;

  case '<':
    if (last_selected_layer > 0) {
      if (setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] >
          0.005) {
        setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] -=
            0.005;
      } else {
        setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] = 0;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_trans[i] > 0.005) {
          setting.detector_trans[i] -= 0.005;
        } else {
          setting.detector_trans[i] = 0;
        }
      }
    }
    ced_needs_redraw = true;
    break;

  case '>':
    if (last_selected_layer > 0) {
      if (setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] <
          1 - 0.005) {
        setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] +=
            0.005;
      } else {
        setting.detector_trans[last_selected_layer - NUMBER_DATA_LAYER] = 1.;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_trans[i] < 1 - 0.005) {
          setting.detector_trans[i] += 0.005;
        } else {
          setting.detector_trans[i] = 1.;
        }
      }
    }
    ced_needs_redraw = true;
    break;

  case 'm':
    if (last_selected_layer > 0) {
      if (setting.detector_cut_angle[last_selected_layer - NUMBER_DATA_LAYER] >
          0) {
        setting.detector_cut_angle[last_selected_layer - NUMBER_DATA_LAYER] -=
            0.5;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_cut_angle[i] > 0) {
          setting.detector_cut_angle[i] -= 0.5;
        }
      }
    }
    ced_needs_redraw = true;
    break;

  case 'M':
    if (last_selected_layer > 0) {
      if (setting.detector_cut_angle[last_selected_layer - NUMBER_DATA_LAYER] <
          360) {
        setting.detector_cut_angle[last_selected_layer - NUMBER_DATA_LAYER] +=
            0.5;
      }
    } else {
      for (int i = 0; i < NUMBER_DETECTOR_LAYER; i++) {
        if (setting.detector_cut_angle[i] < 360) {
          setting.detector_cut_angle[i] += 0.5;
        }
      }
    }
    ced_needs_redraw = true;
    break;

    case 'b': // toggle background color
    ++iBGcolor;
    if (iBGcolor >= sizeof(bgColors) / sizeof(color_t)) {
      glClearColor(userDefinedBGColor[0], userDefinedBGColor[1],
                   userDefinedBGColor[2], userDefinedBGColor[3]);
      iBGcolor = -1;
      printf("using color: %s\n", "user defined");
      ced_needs_redraw = true;
      return;
    } else {
      glClearColor(bgColors[iBGcolor][0], bgColors[iBGcolor][1],
                   bgColors[iBGcolor][2], bgColors[iBGcolor][3]);
      ced_needs_redraw = true;
      printf("using color %u\n", iBGcolor);
    }
    break;
  case 'h':
    toggleHelpWindow();
    break;
  default:
    std::cerr << "Unknown keyboard shortcut: " << key << std::endl;
  }
}

void SpecialKey( int key, int, int ){
   switch (key) {
   case KEY_RIGHT:
    mm.mv.z+=50.;
    break;
   case KEY_LEFT:
    mm.mv.z-=50.;
    break;

   case KEY_UP:
    mm.mv.y+=50.;
    break;
   case KEY_DOWN:
    mm.mv.y-=50.;
    break;

   default:
      return;
   }
   ced_needs_redraw = true;
}


void motion(int x,int y){
    // printf("Mouse moved: %dx%d %f\n",x,y,angle_z);
    if((move_mode == NO_MOVE) || !window_width || !window_height)
      return;

    if(move_mode == TURN_XY){
      //    angle_y=correct_angle(start_angle_y-(x-mouse_x)*180./window_width);
      //    turn_xy((x-mouse_x)*M_PI/window_height,
      //           (y-mouse_y)*M_PI/window_width);
      mm.ha=mm.ha_start+(x-mouse_x)*180./window_width;
      mm.va=mm.va_start+(y-mouse_y)*180./window_height;

      //todo
    } else if (move_mode == ZOOM){
        mm.sf=mm.sf_start+(y-mouse_y)*10./window_height;
        if(mm.sf<0)
  	  mm.sf=0.001;
        else if(mm.sf>2000.)
  	  mm.sf=2000.;
    } else if (move_mode == ORIGIN){
        //cout << "move" << endl;
        /*
        //old code: do not work with rotate
        mm.mv.x=mm.mv_start.x-(x-mouse_x)*WORLD_SIZE/window_width
        mm.mv.y=mm.mv_start.y+(y-mouse_y)*WORLD_SIZE/window_height
        */


//        float grad2rad=3.141*2/360;
        float grad2rad=M_PI*2/360;
        float x_factor_x =  cos(mm.ha*grad2rad);
        float x_factor_y =  cos((mm.va-90)*grad2rad)*cos((mm.ha+90)*grad2rad);
        float y_factor_x =  0;
        float y_factor_y = -cos(mm.va*grad2rad);
        float z_factor_x =  cos((mm.ha-90)*grad2rad);
        float z_factor_y = -cos(mm.ha*grad2rad)*cos((mm.va+90)*grad2rad);

        //float scale_factor=2200/mm.sf/exp(log(window_width*window_height)/2) ;
        float scale_factor=580/mm.sf/exp(log(window_width*window_height)/2.5) ;


        //mm.mv.x=mm.mv_start.x- (x-mouse_x)*WORLD_SIZE/window_width*10*x_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*x_factor_y;
        //mm.mv.y=mm.mv_start.y- (x-mouse_x)*WORLD_SIZE/window_width*10*y_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*y_factor_y;
        //mm.mv.z=mm.mv_start.z - (x-mouse_x)*WORLD_SIZE/window_width*10*z_factor_x - (y-mouse_y)*WORLD_SIZE/window_width*10*z_factor_y;

        mm.mv.x=mm.mv_start.x- scale_factor*(x-mouse_x)*x_factor_x - scale_factor*(y-mouse_y)*x_factor_y;
        mm.mv.y=mm.mv_start.y- scale_factor*(x-mouse_x)*y_factor_x - scale_factor*(y-mouse_y)*y_factor_y;
        mm.mv.z=mm.mv_start.z -scale_factor*(x-mouse_x)*z_factor_x - scale_factor*(y-mouse_y)*z_factor_y;


        //printf("y_factor_x = %f, y_factor_y=%f\n", y_factor_x, y_factor_y);
        //printf("mm.ha = %f, mm.va = %f\n",mm.ha, mm.va);
    }
    ced_needs_redraw = true;
}
