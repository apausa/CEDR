#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <string>

#include <ced.h>
#include <ced_menu.h>
#include <third_party/gl_font.h>

#include "settings.h"
#include "ui/input.h"

using namespace std;

// Owned by glced.cc.
extern float WORLD_SIZE;
extern float FISHEYE_WORLD_SIZE;
extern double fisheye_alpha;
void set_world_size(float length);
void selectFromMenu(int id);

void saveSettings(int slot){
    ofstream file;
    const char *home = getenv("HOME");
    char filename[1000];
    char dirname[1000];

    snprintf(dirname, 1000, "%s/.glced_cfg/", home);
    //if(exists){
       mkdir(dirname,700);
    //}
    snprintf(filename, 1000, "%s/.glced_cfg/settings%i", home, slot);

    //file.open(filename, ios::out | ios::binary);
    file.open(filename);

    if(file.is_open()){
//        file << setting.trans << endl;
//        file << setting.persp << endl;
//        file.close();
        setting.va=mm.va;
        setting.ha=mm.ha;
        setting.win_w=(int)window_width;
        setting.win_h=(int)window_height;
        setting.zoom = mm.sf;
        setting.fisheye_alpha=fisheye_alpha;

        setting.fisheye_world_size = FISHEYE_WORLD_SIZE;
        setting.world_size = WORLD_SIZE;
        //double bgcolor[4];
        glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
        //glGetDoublev(GL_COLOR_CLEAR_VALUE, bgcolor);
        //cout << "bgcolor: " << bgcolor[0] << ", " << bgcolor[1] << ", " << bgcolor[2] << ", "  << bgcolor[3] << "\n" ;

        //file.write((char*)&setting, sizeof(setting));
        file<<"#Config version:"<<std::endl<<VERSION_CONFIG << std::endl;
        file<<"#Transp:"<<std::endl<<setting.trans << std::endl;
        file<<"#Persp:"<<std::endl<<setting.persp  << std::endl;
        file<<"#Anti A:"<<std::endl<<setting.antia<< std::endl;
        file<<"#Light:"<<std::endl<<setting.light<< std::endl;
        //file<<"#Cut angle:"<<std::endl<<setting.cut_angle<< std::endl;
        //file<<"#Trans value:"<<std::endl<<setting.trans_value<< std::endl;
        //for(int i=0;i<CED_MAX_LAYER;i++){
        //    file<<"#Visibility Layer " << i << ":" <<std::endl<<setting.layer[i]<< std::endl;
        //}
        file<<"#Phi projection:"<<std::endl<<setting.phi_projection<< std::endl;
        file<<"#Z projection:"<<std::endl<<setting.z_projection<< std::endl;
        for(int i=0;i<3;i++){
            file<<"#View setting" << i << ":" <<std::endl<<setting.view[i] << std::endl;
        }
        file<<"#Vertical angle:"<<std::endl<<setting.va<< std::endl;
        file<<"#Horiz angle:"<<std::endl<<setting.ha<< std::endl;
        file<<"#Fixed view:"<<std::endl<<setting.fixed_view<< std::endl;
        //file<<"#Z cutting:"<<std::endl<<setting.z_cutting<< std::endl;
        file<<"#Window height:"<<std::endl<<setting.win_h<< std::endl;
        file<<"#Window width:"<<std::endl<<setting.win_w<< std::endl;
        file<<"#Zoom:"<<std::endl<<setting.zoom<< std::endl;
        file<<"#Fisheye_alpha:"<<std::endl<<setting.fisheye_alpha<< std::endl;
        file<<"#World size:"<<std::endl<<setting.world_size<< std::endl;
        file<<"#fisheye world size:"<<std::endl<<setting.fisheye_world_size<< std::endl;
        for(int i=0;i<4;i++){
            file<<"#Background color, value "<< i << ":" << std::endl<<setting.bgcolor[i]<< std::endl;
        }

        file<<"#Show axes:"<<std::endl<<setting.show_axes<< std::endl;
        file<<"#Show fps:"<<std::endl<<setting.fps<< std::endl;

        file<<"#User interface font size"<<std::endl<<setting.font << std::endl;


        for(int i=0;i<CED_MAX_LAYER;i++){
            file<<"#Visibility of data layer " << i << std::endl;
            file<< setting.layer[i] << std::endl;
        }

        for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
            file<<"#Transparency value of detector layer: " << i << std::endl;
            file << setting.detector_trans[i] << std::endl;

            file<<"#Cut angle of detector layer: " << i << std::endl;
            file << setting.detector_cut_angle[i] << std::endl;

            file<<"#Cut z-value of detector layer: " << i << std::endl;
            file << setting.detector_cut_z[i] << std::endl;
        }

        file<<"#Enable detector picking:"<<std::endl<<setting.detector_picking<< std::endl;

        file<<"#Position:"<<std::endl<<
                mm.mv.x<< std::endl <<
                mm.mv.y<< std::endl <<
                mm.mv.z<< std::endl;


        file<<"#Picking marker:"<<std::endl<< setting.picking_highlight << std::endl;

        std::cout << "Save settings to: " << filename << std::endl;

    }else{
        std::cout << "Error open file: " << filename << std::endl;
    }
}

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

void loadSettings(int slot){
    ifstream file;

    const char *home = getenv("HOME");
    char filename[1000];
    snprintf(filename, 1000, "%s/.glced_cfg/settings%i",home, slot);
    //std::cout << "Read config: " << filename << std::endl;
    file.open(filename);

    if(file.is_open()){
        string line;
//        file.read((char*)&setting, sizeof(setting));
            getline(file,line);getline(file,line);
            if(VERSION_CONFIG != atoi(line.c_str())){
                //std::cout << "WARNING: Cant read configfile (" << filename << ") please delete or rename it" << std::endl;
                std::cout << "WARNING: Cant read configfile (" << filename << ") version does not match! Please delete or rename the file" << std::endl;
                defaultSettings();
                return;
            } else{
                getline(file,line);getline(file,line);
                setting.trans=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.persp=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.antia=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.light=atoi(line.c_str());



                getline(file,line);getline(file,line);
                setting.phi_projection=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.z_projection=atoi(line.c_str());

                for(int i=0;i<3;i++){
                    getline(file,line);getline(file,line);
                    setting.view[i]=atof(line.c_str());
                }

                getline(file,line);getline(file,line);
                setting.va=atof(line.c_str());
                getline(file,line);getline(file,line);
                setting.ha=atof(line.c_str());

                getline(file,line);getline(file,line);
                setting.fixed_view=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.win_h=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.win_w=atoi(line.c_str());
                if(setting.win_w == 0 || setting.win_h == 0){
                    setting.win_w = setting.win_h = 500;
                }

                getline(file,line);getline(file,line);
                setting.zoom=atof(line.c_str());

                getline(file,line);getline(file,line);
                setting.fisheye_alpha=atof(line.c_str());

                getline(file,line);getline(file,line);
                setting.world_size=atof(line.c_str());

                getline(file,line);getline(file,line);
                setting.fisheye_world_size=atof(line.c_str());

                for(int i=0;i<4;i++){
                    getline(file,line);getline(file,line);
                    setting.bgcolor[i] = atof(line.c_str());
                }

                getline(file,line);getline(file,line);
                setting.show_axes=atoi(line.c_str());

                getline(file,line);getline(file,line);
                setting.fps=atoi(line.c_str());


                getline(file,line);getline(file,line);
                setting.font = atoi(line.c_str());

                for(int i=0;i<CED_MAX_LAYER;i++){
                    getline(file,line);getline(file,line);
                    setting.layer[i]=atoi(line.c_str());
                }

                for(int i=0;i<NUMBER_DETECTOR_LAYER;i++){
                    getline(file,line);getline(file,line);
                    setting.detector_trans[i]=atof(line.c_str());

                    getline(file,line);getline(file,line);
                    setting.detector_cut_angle[i]=atof(line.c_str());

                    getline(file,line);getline(file,line);
                    setting.detector_cut_z[i]=atof(line.c_str());
                }

                getline(file,line);getline(file,line);
                setting.detector_picking = atoi(line.c_str());



                getline(file,line);getline(file,line);
                mm.mv.x = atof(line.c_str());
                getline(file,line);
                mm.mv.y = atof(line.c_str());
                getline(file,line);
                mm.mv.z = atof(line.c_str());


                getline(file,line);getline(file,line);
                setting.picking_highlight = atoi(line.c_str());




            //set_bg_color(setting.bgcolor[0],setting.bgcolor[1],setting.bgcolor[2],setting.bgcolor[3]);
            std::cout << "Read settings from: " << filename << std::endl;

	    setting.autoshot_scale=1;
            setting.autoshot=false;

        }

    }else{ //set to default
        std::cout << "WARNING: Failed to read settings from: " << filename << std::endl;
        defaultSettings();
    }

    mm.va=setting.va;
    mm.ha=setting.ha;
    mm.sf = setting.zoom;

    fisheye_alpha=setting.fisheye_alpha;


    FISHEYE_WORLD_SIZE = setting.fisheye_world_size;
    WORLD_SIZE=setting.world_size;

    //reshape(setting.win_w, setting.win_h);
}
