/* Action dispatch for CED's event display server.
 * Split out of glced.cc: this is the handler invoked for every
 * keyboard shortcut and scroll-wheel action. */

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <cstring>
#include <iostream>

#include <ced.h>
#include <ced_actions.h>
#include <ced_cli.h>
#include <ced_config.h>

#include "ui/input.h"
#include "render.h"
#include "utils/helpers.h"

using namespace std;

extern float WORLD_SIZE;
extern float FISHEYE_WORLD_SIZE;
extern double fisheye_alpha;

CEDsettings setting_old[5];

static void toggle_layer(unsigned l){
    if(l > CED_MAX_LAYER-1){ return; }

    if(setting.layer[l]){
        setting.layer[l]=false;
    }else{
        setting.layer[l]=true;
    }

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
    static float mm_ha_backup;
    static float mm_va_backup;
    static int graphic_2_backup;

    if(id != UNDO){
        setting_old[4]=setting_old[3];
        setting_old[3]=setting_old[2];
        setting_old[2]=setting_old[1];
        setting_old[1]=setting_old[0];
        setting_old[0]=setting;
    }

    switch(id){
        case VIEW_RESET:
            setting.phi_projection = false; // no phi projection
            setting.z_projection=false; // no phi projection;
            mm=mm_reset;
            fisheye_alpha=0;
            setting.fixed_view=false;
            set_world_size(DEFAULT_WORLD_SIZE );
            break;

        case CED_RESET:
            if((setting.trans == true && setting.persp == false) || (setting.trans == false && setting.persp == true)){
                selectFromMenu(GRAFIC_PERSP); //switch persp on in new view, switch persp off in classic view
            }
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
            fisheye_alpha=0;
            setting.fixed_view=false;
            set_world_size(DEFAULT_WORLD_SIZE );
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
            if(setting.fixed_view){ break;}

            mm.ha=180.;
            mm.va=0.;
            break;

        case VIEW_SIDE:
            if(setting.fixed_view){ break;}

                mm.ha=90.;
                mm.va=0.;

            break;

        case TOGGLE_PHI_PROJECTION:
            if(setting.phi_projection){ //turn projection off
                setting.phi_projection=false;

                copySetting(setting, backup_setting, "cut angle");
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting

                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;

            }else{ //turn projection on
                if(setting.z_projection){
                    selectFromMenu(TOGGLE_Z_PROJECTION);
                }

                copySetting(backup_setting, setting, "cut angle");

                setting.phi_projection=true;

                graphic_2_backup=setting.persp;

                if(setting.persp==1){selectFromMenu(GRAFIC_PERSP); }


                for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
                    setting.detector_cut_angle[i]=180;
                }
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;
                mm.ha=90.;
                mm.va=0.;

                setting.fixed_view=true;
            }

            break;

        case TOGGLE_Z_PROJECTION:
            if(setting.z_projection){ //turn projection off

                setting.z_projection=false;

                copySetting(setting, backup_setting, "cut z");
                if(graphic_2_backup != setting.persp){selectFromMenu(GRAFIC_PERSP); } //restore persp setting


                mm.ha = mm_ha_backup;
                mm.va = mm_va_backup;

                setting.fixed_view=false;
            }else{ //turn projection on

                if(setting.phi_projection){selectFromMenu(TOGGLE_PHI_PROJECTION);}

                copySetting(backup_setting, setting, "cut z");

                setting.z_projection=true;

                for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
                    setting.detector_cut_z[i]=-10;
                }

                graphic_2_backup=setting.persp;

                if(setting.persp==true){selectFromMenu(GRAFIC_PERSP); }


               //side view
                mm_ha_backup=mm.ha;
                mm_va_backup = mm.va;

                mm.ha=180.;
                mm.va=0.;

                setting.fixed_view=true;
            }

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
            break;

        case VIEW_ZOOM_OUT:
            mm.sf -= mm.sf*50.0/window_height;
            break;

        case LAYER_ALL:
            anz=0;
            for(int i=0;i<NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   toggle_layer(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                for(int i=0;i<NUMBER_DATA_LAYER;i++){
                   toggle_layer(i);
                }
            }
            break;

        case DETECTOR_ALL:
            anz=0;
            for(int i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){ //try to turn all layers on
                if(!isLayerVisible(i)){
                   toggle_layer(i);
                   anz++;
                }
            }
            if(anz == 0){ //turn all layers off
                for(int i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
                   toggle_layer(i);
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
            break;

        case GRAFIC_LIGHT:
            if(setting.light == true){
                setting.light=false;
                glDisable(GL_LIGHTING);
            }else{
                 setting.light = true;

                 //TODO: CHANGE IT
                 GLfloat light0_spec[] = {1, 1, 1, 0.5};
                 GLfloat light0_ambi[] = {1, 1, 1, 0.5};
                 GLfloat light0_diff[] = {1, 1, 1, 0.5};

                 GLfloat light0_pos[] = {20000, 20000, 20000};

            glBegin(GL_QUADS);
            glVertex3d(2000,2000,20000);
            glVertex3d(2500,2000,20000);
            glVertex3d(2000,2500,20000);
            glVertex3d(2000,2000,20500);
            glEnd();

                 glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);
                 glLightfv(GL_LIGHT1, GL_DIFFUSE, light0_ambi);
                 glLightfv(GL_LIGHT2, GL_AMBIENT, light0_diff);

                 glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
                 glLightfv(GL_LIGHT1, GL_POSITION, light0_pos);
                 glLightfv(GL_LIGHT2, GL_POSITION, light0_pos);

                 glColorMaterial ( GL_FRONT_AND_BACK, GL_EMISSION ) ;
                 glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE) ;
                 glEnable (GL_COLOR_MATERIAL) ;


                 glEnable(GL_NORMALIZE);

                 glEnable(GL_LIGHTING);
                 glEnable(GL_LIGHT0);

                 glEnable(GL_DEPTH_TEST);

                 glMatrixMode(GL_MODELVIEW);
            }
            break;

        case GRAFIC_PERSP:
            if(setting.persp == true){
                setting.persp = false;

                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }else{
                setting.persp = true;
                reshape((int)window_width, (int)window_height); //hack, call resize function to overwrite perspectivic settings
            }
            break;

    }

    ced_needs_redraw = true;

}
