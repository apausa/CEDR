#include <iostream>

#include <ced.h>
#include <ced_menu.h>
#include <third_party/gl_font.h>

#include "settings.h"
#include "ui/input.h"

// Owned by glced.cc.
extern float WORLD_SIZE;
extern float FISHEYE_WORLD_SIZE;
extern double fisheye_alpha;
void set_world_size(float length);
void selectFromMenu(int id);

void defaultSettings(void){
        setting.trans=true;
        setting.light=false;
        setting.antia=false;
        //setting.cut_angle=180;
        //setting.trans_value=0.8;
        //setting.z_cutting=7000;


        setting.win_w=500;
        setting.win_h=500;
        setting.show_axes=true;
        setting.fps=false;
        setting.persp=true;
        setting.picking_highlight=false;





        for(int i=0;i < 4; i++){
            //setting.bgcolor[i]=0; //black
            setting.bgcolor[i]=1; //white
        }


        setting.font=FONT_M;

        for(int i=0; i < CED_MAX_LAYER; i++){
            setting.layer[i]=true; // turn all layers on
        }

        for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
            setting.detector_trans[i] =0.8;
            setting.detector_cut_angle[i] = 0;//180;
            setting.detector_cut_z[i] = 7000;
        }

            setting.phi_projection = false; // no phi projection
            setting.z_projection=false; // no phi projection;
            //mm.sf = fisheye_alpha > 0 ? mm.sf*8.0: mm.sf;
            //fisheye_alpha=0;
            setting.fixed_view=false;
            //update_cut_angle_menu();

            set_world_size(DEFAULT_WORLD_SIZE );


        //mm=mm_reset;
        //setting.va=mm.va;
        //setting.ha=mm.ha;


       //setting.zoom=0.072033;
       //mm.sf = setting.zoom;
//       fisheye_alpha=setting.fisheye_alpha;

//        FISHEYE_WORLD_SIZE = setting.fisheye_world_size;
//        WORLD_SIZE=setting.world_size;
//        selectFromMenu(VIEW_RESET);

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


            setting.va=mm.va;
            setting.ha=mm.ha;
            setting.zoom=mm.sf;
            setting.fisheye_alpha=fisheye_alpha;

            setting.fisheye_world_size= FISHEYE_WORLD_SIZE ;
            setting.world_size= WORLD_SIZE;

            setting.autoshot=false;
            setting.autoshot_scale=1;

            std::cout << "Set options to default settings" << std::endl;
}
