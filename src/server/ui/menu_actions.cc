/* Menu-action dispatch for CED's event display server.
 * Split out of glced.cc: this is the handler invoked for every
 * menu click and menu-driven keyboard shortcut. */

#include <sys/types.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <ced.h>
#include <ced_cli.h>
#include <ced_config.h>
#include <ced_menu.h>

#include "io/screenshot.h"
#include "ui/input.h"
#include "ui/menu.h"
#include "ui/selection.h"

using namespace std;

extern CED_PopUpMenu *popupmenu;

extern int last_selected_layer;
extern int socket_fd;
extern bool client_connected;

extern float userDefinedBGColor[];
extern float WORLD_SIZE;
extern float FISHEYE_WORLD_SIZE;
extern double fisheye_alpha;

extern void (*idle_func)(void);
void idle(void);

int isLayerVisible(int x);
void set_world_size(float length);
void set_bg_color(float one, float two, float three, float four);
void toggle_layer(unsigned l);
void reshape(int w, int h);

CEDsettings setting_old[5];

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
