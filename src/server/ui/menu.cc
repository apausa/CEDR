#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>

#include <ced.h>
#include <ced_menu.h>

#include "menu.h"
#include "overlay.h"
#include "input.h"
#include "layers.h"
#include "selection.h"
#include "utils/helpers.h"

using namespace std;

// Owned by glced.cc.
extern int last_selected_layer;
extern int selected_layer;
extern bool select_nothing;
extern Point pick_point;
extern Point pre_pick_point;
extern CED_Menu *ced_menu;
extern CED_PopUpMenu *popupmenu;
extern CED_SubSubMenu *detectorlayermenu;
extern CED_SubSubMenu *datalayermenu;
extern double fisheye_alpha;
extern int showHelp;
extern float userDefinedBGColor[];

//for new angles add the new angle to this list and to define in ced_menu.h
static int available_cutangles[]={0,30,45,90,100,135,120,150,170,180,190,200,220,240,260,270,280,290,310,330,340};

void buildPopUpMenu(int x, int y){
    //static GLfloat p_pre_x=0, p_pre_y=0, p_pre_z=0;
    char tmp[200];

    GLfloat p_x, p_y, p_z;
    int id, layer, type;

    //delete the old one first!!!

    popupmenu=new CED_PopUpMenu("");

    buildLayerMenus();
    popupmenu->addItem(datalayermenu);
    popupmenu->addItem(detectorlayermenu);
    popupmenu->addItem(new CED_SubSubMenu("---",0));

    selected_layer=-1;
    if(!find_selected_object(x,y,&p_x,&p_y,&p_z, &id, &layer, &type)){ //if ==1 found hit, else clicked on background
        //cout << "TODO: ID: " << id << endl;
        //cout << "PICK_HIT" << endl;
        //find_selected_object(popupmenu->x_click,popupmenu->y_click,&pick_point.x,&pick_point.y,&pick_point.z, NULL, NULL, NULL);
        select_nothing=false;
        pick_point.x=p_x;pick_point.y=p_y;pick_point.z=p_z;


        if(type == 0){
            last_selected_layer=layer;
            //popupmenu=new CED_PopUpMenu("Select datapoint");

            popupmenu->addItem(new CED_SubSubMenu("Selected datapoint:",0));
            sprintf(tmp,"Coordinates: (%.1f, %.1f, %1.f)",p_x,p_y,p_z);
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            sprintf(tmp,"ID: %i",id);
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            sprintf(tmp,"Distance previous selected hit: %.2f",pow(pow(pre_pick_point.x-p_x,2)+pow(pre_pick_point.y-p_y,2)+pow(pre_pick_point.z-p_z,2),0.5));
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            sprintf(tmp,"Center object");
            popupmenu->addItem(new CED_SubSubMenu(tmp,CENTER_HIT));
            sprintf(tmp,"Pick object");
            popupmenu->addItem(new CED_SubSubMenu(tmp,PICK_HIT));
            sprintf(tmp,"Hide layer %i: %s)",layer,layerDescription[layer] );
            popupmenu->addItem(new CED_SubSubMenu(tmp,LAYER_0+layer));

            pre_pick_point.x=p_x;
            pre_pick_point.y=p_y;
            pre_pick_point.z=p_z;
        }else if(type == 1){
            //popupmenu=new CED_PopUpMenu("Select detector component");


            select_nothing=true;
            last_selected_layer=layer;
            selected_layer=layer;
            sprintf(tmp, "Selected detector: %s (Layer: %i)", layerDescription[layer], layer);
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));

            sprintf(tmp,"Hide layer %i",layer);
            popupmenu->addItem(new CED_SubSubMenu(tmp, layer-NUMBER_DATA_LAYER+DETECTOR1));

            //sprintf(tmp,"Cut detector at this layer",layer, layerDescription[layer]);
            //popupmenu->addItem(new CED_SubSubMenu(tmp, layer-NUMBER_DATA_LAYER+DETECTOR1));


            sprintf(tmp,"Coordinates: (%.1f, %.1f, %1.f)",p_x,p_y,p_z);
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            sprintf(tmp,"ID: %i",id);
            popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            //sprintf(tmp,"Distance previous selected object: %.2f",pow(pow(p_pre_x-p_x,2)+pow(p_pre_y-p_y,2)+pow(p_pre_z-p_z,2),0.5));
            //popupmenu->addItem(new CED_SubSubMenu(tmp,0));
            sprintf(tmp,"Center object");
            popupmenu->addItem(new CED_SubSubMenu(tmp,CENTER_HIT));
            sprintf(tmp,"Pick object");
            popupmenu->addItem(new CED_SubSubMenu(tmp,PICK_HIT));
            //sprintf(tmp,"Hide layer %i: %s",layer, layerDescription[layer]);
            //popupmenu->addItem(new CED_SubSubMenu(tmp, layer-NUMBER_DATA_LAYER+DETECTOR1));


            snprintf(tmp,199,"Phi cut (%.0f)",setting.detector_cut_angle[layer-NUMBER_DATA_LAYER]);
            CED_SubSubMenu *phicuts=new CED_SubSubMenu(tmp);

            unsigned i;
            char str[200];
            for(i=0; (unsigned)i < sizeof(available_cutangles)/sizeof(available_cutangles[0]); i++){
                    sprintf(str,"Cut of %i degree in phi", available_cutangles[i]);
                    phicuts->addItem(new CED_SubSubMenu(str,  LAYER_CUT_ANGLE0+available_cutangles[i]));
                    //glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
            }
            popupmenu->addItem(phicuts);

            char new_tmp[200];
            snprintf(new_tmp,199,"Z-cut (%.0f)",setting.detector_cut_z[layer-NUMBER_DATA_LAYER]);
            CED_SubSubMenu *zcuts=new CED_SubSubMenu(new_tmp);
            zcuts->addItem(new CED_SubSubMenu("Cut at z=-6000", LAYER_CUT_Z_M6000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=-4000", LAYER_CUT_Z_M4000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=-2000", LAYER_CUT_Z_M2000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=0",     LAYER_CUT_Z_0000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=2000",  LAYER_CUT_Z_2000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=4000",  LAYER_CUT_Z_4000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=6000",  LAYER_CUT_Z_6000));
            zcuts->addItem(new CED_SubSubMenu("Cut at z=7000",  LAYER_CUT_Z_7000));
            popupmenu->addItem(zcuts);


            //CED_SubMenu *phicuts=new CED_SubMenu("Cuts");
            //cuts->addItem(new CED_SubSubMenu("Cut at z=0",  0));
            //cuts->addItem(new CED_SubSubMenu("Cut at z=3000",  0));
            //cuts->addItem(new CED_SubSubMenu("Cut at z=5000",  0));




            snprintf(new_tmp,199,"Transparency (%.0f)",100*setting.detector_trans[layer-NUMBER_DATA_LAYER]);
            CED_SubSubMenu *trans=new CED_SubSubMenu(new_tmp);
            trans->addItem(new CED_SubSubMenu("    0%",LAYER_TRANS0));
            trans->addItem(new CED_SubSubMenu("  40%", LAYER_TRANS40));
            trans->addItem(new CED_SubSubMenu("  60%", LAYER_TRANS60));
            trans->addItem(new CED_SubSubMenu("  70%", LAYER_TRANS70));
            trans->addItem(new CED_SubSubMenu("  80%", LAYER_TRANS80));
            trans->addItem(new CED_SubSubMenu("  90%", LAYER_TRANS90));
            trans->addItem(new CED_SubSubMenu("  95%", LAYER_TRANS95));
            trans->addItem(new CED_SubSubMenu("100%",  LAYER_TRANS100));
            popupmenu->addItem(trans);



            //pre_pick_point.x=p_x;
            //pre_pick_point.y=p_y;
            //pre_pick_point.z=p_z;
        }
    }else{
           select_nothing=true;


            last_selected_layer=-1;
        //popupmenu=new CED_PopUpMenu("Change background color to:");

        popupmenu->addItem(new CED_SubSubMenu("Change background color to:",0));
        sprintf(tmp,"%s",CED_BGCOLOR_OPTION1_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION1));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION2_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION2));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION3_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION3));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION4_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION4));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION5_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION5));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION6_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION6));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION7_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION7));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION8_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION8));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION9_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION9));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION10_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION10));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION11_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION11));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION12_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION12));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION13_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION13));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION14_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION14));

        sprintf(tmp,"%s",CED_BGCOLOR_OPTION15_NAME);
        popupmenu->addItem(new CED_SubSubMenu(tmp,BGCOLOR_OPTION15));

//
//
//      if(userDefinedBGColor[0] >= 0){ //is set
//          settings->addItem(new CED_SubSubMenu("User defined",BGCOLOR_USER));
//      }

    }

    int height = font_get_height(setting.font);
    int width = font_get_width(setting.font, "A") * 20;

    int pos_y=popupmenu->size()*height;

    popupmenu->isExtend=true;

    if( (x + width+10) > window_width){
        popupmenu->x_start=x-width-10;
        popupmenu->x_end=x-10;
    }else{
        popupmenu->x_start=x;
        popupmenu->x_end=x+width;
    }

    if( (y+pos_y) > window_height){
        popupmenu->y_start=y-pos_y;
        popupmenu->y_end=y-pos_y+height+1;
    }else{
        popupmenu->y_start=y;
        popupmenu->y_end=y+height+1;
    }

    popupmenu->y_click=y;
    popupmenu->x_click=x;
   // cout << "TODO: x,y: " << x << ", " << y << endl;
}


std::string formatSubmenuEntry(int iLayer, const char key,
                               const char *description, size_t max_len) {
  std::stringstream sstr;
  sstr << (isLayerVisible(iLayer) ? "[X]" : "[ ]") << " " << std::setfill('0')
       << std::setw(2) << iLayer;

  // truncate the string such that the description has length max_len
  // i.e. add the other chars to the length of the final string
  int padding = 7;

  if (key != '0') {
    sstr << " [" << key << "]";
    padding = 11;
  }
  sstr << ": " << description;

  return truncateTo(sstr.str(), max_len + padding);
}

void buildLayerMenus(void){
    //std::cout << "enter buildLayerMenus" << std::endl;
    //if(ced_menu != NULL && detectorlayermenu != NULL && datalayermenu != NULL){
    //    delete ced_menu;
    //}

    detectorlayermenu=new CED_SubSubMenu("Detector layers",0);
    datalayermenu=new CED_SubSubMenu("Data layers",0);
    int i;
    constexpr unsigned max=150;
    for(i=0;i<NUMBER_POPUP_LAYER;i++){
        datalayermenu->addItem(new CED_SubSubMenu(formatSubmenuEntry(i, layer_keys[i], layerDescription[i], max), LAYER_0+i));
    }
    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
        detectorlayermenu->addItem(new CED_SubSubMenu(formatSubmenuEntry(i, '0', layerDescription[i], max), DETECTOR1+i-NUMBER_DATA_LAYER));
    }


    //std::cout << "leave buildLayerMenus" << std::endl;
    buildMainMenu();
}
void buildMainMenu(void){
    //std::cout << "build main menu" << std::endl;
    ced_menu=new CED_Menu();
    //buildLayerMenus();


    char str[200];
    unsigned i;

    //layers
    CED_SubMenu *layers=new CED_SubMenu("Layers");

    //buildLayerMenus();

    if(setting.show_axes == true){
        layers->addItem(new CED_SubSubMenu("[X] Axis", AXES));
    }else{
        layers->addItem(new CED_SubSubMenu("[ ] Axis", AXES));
    }

    layers->addItem(new CED_SubSubMenu("---", AXES));
    bool result=true;
    for(int ii=0;ii<NUMBER_DATA_LAYER;ii++){
        if(setting.layer[ii] == false){
            result=false;
            break;
        }
    }
    if(result){
        layers->addItem(new CED_SubSubMenu("[X] Show/Hide all data Layers [`]", LAYER_ALL));
    }else{
        layers->addItem(new CED_SubSubMenu("[ ] Show/Hide all data Layers [`]", LAYER_ALL));
    }

    layers->addItem(datalayermenu);

    layers->addItem(new CED_SubSubMenu("---", AXES));

    result=true;
    for(i=NUMBER_DATA_LAYER;i<NUMBER_DETECTOR_LAYER+NUMBER_DATA_LAYER;i++){
        if(setting.layer[i] == false){
            result=false;
            break;
        }
    }
    if(result){
        layers->addItem(new CED_SubSubMenu("[X] Show/Hide complete detector", DETECTOR_ALL));
    }else{
        layers->addItem(new CED_SubSubMenu("[ ] Show/Hide complete detector", DETECTOR_ALL));
    }

    layers->addItem(detectorlayermenu);
    ced_menu->addSubMenu(layers);

    double tmptrans=setting.detector_trans[0];
    for(i=1;i<NUMBER_DETECTOR_LAYER;i++){
        if(setting.detector_trans[i] != tmptrans){
            tmptrans=-100;
            break;
        }
    }

    CED_SubMenu *trans=new CED_SubMenu("Transparency");
    if(tmptrans == 0){
        trans->addItem(new CED_SubSubMenu("[X]   0%",TRANS0));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]   0%",TRANS0));
    }

    if(tmptrans == 0.40){
        trans->addItem(new CED_SubSubMenu("[X]  40%",TRANS40));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  40%",TRANS40));
    }

    if(tmptrans == 0.60){
        trans->addItem(new CED_SubSubMenu("[X]  60%",TRANS60));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  60%",TRANS60));
    }

    if(tmptrans == 0.70){
        trans->addItem(new CED_SubSubMenu("[X]  70%",TRANS70));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  70%",TRANS70));
    }

    if(tmptrans == 0.80){
        trans->addItem(new CED_SubSubMenu("[X]  80%",TRANS80));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  80%",TRANS80));
    }


    if(tmptrans == 0.90){
        trans->addItem(new CED_SubSubMenu("[X]  90%",TRANS90));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  90%",TRANS90));
    }


    if(tmptrans == 0.95){
        trans->addItem(new CED_SubSubMenu("[X]  95%",TRANS95));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ]  95%",TRANS95));
    }


    if(tmptrans == 1.00){
        trans->addItem(new CED_SubSubMenu("[X] 100%",TRANS100));
    }else{
        trans->addItem(new CED_SubSubMenu("[ ] 100%",TRANS100));
    }
    ced_menu->addSubMenu(trans);

    CED_SubMenu *camera=new CED_SubMenu("Camera");
    camera->addItem(new CED_SubSubMenu("Reset view [r]", VIEW_RESET));
    camera->addItem(new CED_SubSubMenu("Reset CED  [R]", CED_RESET));
    camera->addItem(new CED_SubSubMenu("Front view [f]", VIEW_FRONT));
    camera->addItem(new CED_SubSubMenu("Side view [s]",  VIEW_SIDE));

    camera->addItem(new CED_SubSubMenu("---", 0));
    if(setting.phi_projection==true){
        camera->addItem(new CED_SubSubMenu("[X] Toggle side view projection [S]", TOGGLE_PHI_PROJECTION));
    }else{
        camera->addItem(new CED_SubSubMenu("[ ] Toggle side view projection [S]", TOGGLE_PHI_PROJECTION));
    }
    if(setting.z_projection==true){
        camera->addItem(new CED_SubSubMenu("[X] Toggle front view projection [F]", TOGGLE_Z_PROJECTION));
    }else{
        camera->addItem(new CED_SubSubMenu("[ ] Toggle front view projection [F]", TOGGLE_Z_PROJECTION));
    }
    if(fisheye_alpha > 0){
        camera->addItem(new CED_SubSubMenu("[X] Toggle fisheye projection [v]",VIEW_FISHEYE));
    }else{
        camera->addItem(new CED_SubSubMenu("[ ] Toggle fisheye projection [v]",VIEW_FISHEYE));
    }

    camera->addItem(new CED_SubSubMenu("---", 0));
    camera->addItem(new CED_SubSubMenu("Zoom in [+]", VIEW_ZOOM_IN));
    camera->addItem(new CED_SubSubMenu("Zoom out [-]", VIEW_ZOOM_OUT));
    ced_menu->addSubMenu(camera);




    double tmpcut=setting.detector_cut_angle[0];
    for(i=1;i<NUMBER_DETECTOR_LAYER;i++){
        if(setting.detector_cut_angle[i] != tmpcut){
            tmpcut=-100;
            break;
        }
    }

    CED_SubMenu *cuts=new CED_SubMenu("Cuts");
    for(i=0; (unsigned)i < sizeof(available_cutangles)/sizeof(available_cutangles[0]); i++){
            if(available_cutangles[i] == tmpcut){
                sprintf(str,"[X] Cut of %i degree in phi", available_cutangles[i]);
            }else{
                sprintf(str,"[ ] Cut of %i degree in phi", available_cutangles[i]);
            }
            cuts->addItem(new CED_SubSubMenu(str,  CUT_ANGLE0+available_cutangles[i]));
            //glutChangeToMenuEntry(i+1, str,  CUT_ANGLE0+i);
    }

    //CED_SubSubMenu *zcuts=new CED_SubSubMenu("Z cut");

    tmpcut=setting.detector_cut_z[0];
    for(i=1;i<NUMBER_DETECTOR_LAYER;i++){
        if(setting.detector_cut_z[i] != tmpcut){
            tmpcut=-99999999;
            break;
        }
    }

    cuts->addItem(new CED_SubSubMenu("---", 0));
    if(tmpcut == -6000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=-6000", CUT_Z_M6000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=-6000", CUT_Z_M6000));
    }

    if(tmpcut == -4000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=-4000", CUT_Z_M4000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=-4000", CUT_Z_M4000));
    }

    if(tmpcut == -2000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=-2000", CUT_Z_M2000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=-2000", CUT_Z_M2000));
    }

    if(tmpcut == 0){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=0", CUT_Z_0000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=0", CUT_Z_0000));
    }
    if(tmpcut == 2000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=2000", CUT_Z_2000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=2000", CUT_Z_2000));
    }
    if(tmpcut == 4000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=4000", CUT_Z_4000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=4000", CUT_Z_4000));
    }

    if(tmpcut == 6000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=6000", CUT_Z_6000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=6000", CUT_Z_6000));
    }

    if(tmpcut == 7000){
        cuts->addItem(new CED_SubSubMenu("[X] Cut at z=7000", CUT_Z_7000));
    }else{
        cuts->addItem(new CED_SubSubMenu("[ ] Cut at z=7000", CUT_Z_7000));
    }

    ced_menu->addSubMenu(cuts);


    CED_SubMenu *settings=new CED_SubMenu("Graphic");
    if(setting.trans==false && setting.persp==false){
        settings->addItem(new CED_SubSubMenu("[X] Graphic low",GRAFIC_LOW));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Graphic low",GRAFIC_LOW));
    }


    if(setting.trans==true && setting.persp==true){
        settings->addItem(new CED_SubSubMenu("[X] Graphic high",GRAFIC_HIGH));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Graphic high",GRAFIC_HIGH));
    }

    settings->addItem(new CED_SubSubMenu("---",0));
    if(setting.persp){
        settings->addItem(new CED_SubSubMenu("[X] Toggle perspective",GRAFIC_PERSP));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Toggle perspective",GRAFIC_PERSP));
    }
    if(setting.trans){
        settings->addItem(new CED_SubSubMenu("[X] Toggle wireframe",GRAFIC_TRANS));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Toggle wireframe",GRAFIC_TRANS));
    }
    if(setting.light){
        settings->addItem(new CED_SubSubMenu("[X] Light", GRAFIC_LIGHT));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Light", GRAFIC_LIGHT));
    }
    if(setting.antia){
        settings->addItem(new CED_SubSubMenu("[X] Anti Aliasing", GRAFIC_ALIAS));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Anti Aliasing", GRAFIC_ALIAS));
    }

    if(setting.picking_highlight){
        settings->addItem(new CED_SubSubMenu("[X] Picking marker", PICKING_MARKER));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Picking marker", PICKING_MARKER));
    }


    settings->addItem(new CED_SubSubMenu("Fade far objects",GRAFIC_FOG));
    settings->addItem(new CED_SubSubMenu("Deepbuffer", GRAFIC_BUFFER));

    settings->addItem(new CED_SubSubMenu("---",0));
    if(setting.detector_picking){
        settings->addItem(new CED_SubSubMenu("[X] Detector picking", TOGGLE_DETECTOR_PICKING));
    }else{
        settings->addItem(new CED_SubSubMenu("[ ] Detector picking", TOGGLE_DETECTOR_PICKING));
    }
    settings->addItem(new CED_SubSubMenu("---",0));

    CED_SubSubMenu *font=new CED_SubSubMenu("Text font size ");
    if(setting.font == FONT_S){
        font->addItem(new CED_SubSubMenu("[X] Small",FONT0));
    }else{
        font->addItem(new CED_SubSubMenu("[ ] Small",FONT0));
    }
    if(setting.font == FONT_M){
        font->addItem(new CED_SubSubMenu("[X] Medium",FONT1));
    }else{
        font->addItem(new CED_SubSubMenu("[ ] Medium",FONT1));
    }
    if(setting.font == FONT_L){
        font->addItem(new CED_SubSubMenu("[X] Large",FONT2));
    }else{
        font->addItem(new CED_SubSubMenu("[ ] Large",FONT2));
    }
    settings->addItem(font);




    CED_SubSubMenu *background=new CED_SubSubMenu("Change background color");
    //sprintf(str,"Change background color to: %s",CED_BGCOLOR_OPTION1_NAME);
    //background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION1));
    sprintf(str,"%s",CED_BGCOLOR_OPTION2_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION2));
    sprintf(str,"%s",CED_BGCOLOR_OPTION3_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION3));
    sprintf(str,"%s",CED_BGCOLOR_OPTION4_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION4));
    sprintf(str,"%s",CED_BGCOLOR_OPTION5_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION5));
    sprintf(str,"%s",CED_BGCOLOR_OPTION6_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION6));
    sprintf(str,"%s",CED_BGCOLOR_OPTION7_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION7));
    sprintf(str,"%s",CED_BGCOLOR_OPTION8_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION8));
    sprintf(str,"%s",CED_BGCOLOR_OPTION9_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION9));
    sprintf(str,"%s",CED_BGCOLOR_OPTION10_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION10));
    sprintf(str,"%s",CED_BGCOLOR_OPTION11_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION11));
    sprintf(str,"%s",CED_BGCOLOR_OPTION12_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION12));
    sprintf(str,"%s",CED_BGCOLOR_OPTION13_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION13));
    sprintf(str,"%s",CED_BGCOLOR_OPTION14_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION14));
    sprintf(str,"%s",CED_BGCOLOR_OPTION15_NAME);
    background->addItem(new CED_SubSubMenu(str,BGCOLOR_OPTION15));


    if(userDefinedBGColor[0] >= 0){ //is set
        background->addItem(new CED_SubSubMenu("User defined",BGCOLOR_USER));
    }


    settings->addItem(new CED_SubSubMenu("---",0));
    settings->addItem(background);

    ced_menu->addSubMenu(settings);

    char tmp[200];
    CED_SubSubMenu *screenshot=new CED_SubSubMenu("Save screenshot");
    sprintf(tmp,"original size (%i x %i)", int(setting.win_w), int(setting.win_h));
    screenshot->addItem(new CED_SubSubMenu(tmp,SAVE_IMAGE1));

    sprintf(tmp,"large (%i x %i)", int(4*setting.win_w), int(4*setting.win_h));
    screenshot->addItem(new CED_SubSubMenu(tmp,SAVE_IMAGE4));

    sprintf(tmp,"very large (%i x %i)", int(10*setting.win_w), int(10*setting.win_h));
    screenshot->addItem(new CED_SubSubMenu(tmp,SAVE_IMAGE10));

    sprintf(tmp,"very very large (%i x %i)", int(20*setting.win_w), int(20*setting.win_h));
    screenshot->addItem(new CED_SubSubMenu(tmp,SAVE_IMAGE20));

    sprintf(tmp,"extrem large (%i x %i)", int(100*setting.win_w), int(100*setting.win_h));
    screenshot->addItem(new CED_SubSubMenu(tmp,SAVE_IMAGE100));

    CED_SubSubMenu *autoshot=new CED_SubSubMenu("A. shot scale");
    sprintf(tmp,"original size (%i x %i)", int(setting.win_w), int(setting.win_h));
    autoshot->addItem(new CED_SubSubMenu(tmp,AUTOSHOT_IMAGE1));

    sprintf(tmp,"large size (%i x %i)", int(4*setting.win_w), int(4*setting.win_h));
    autoshot->addItem(new CED_SubSubMenu(tmp,AUTOSHOT_IMAGE4));

    sprintf(tmp,"very large size (%i x %i)", int(10*setting.win_w), int(10*setting.win_h));
    autoshot->addItem(new CED_SubSubMenu(tmp,AUTOSHOT_IMAGE10));

    sprintf(tmp,"very very large size (%i x %i)", int(20*setting.win_w), int(20*setting.win_h));
    autoshot->addItem(new CED_SubSubMenu(tmp,AUTOSHOT_IMAGE20));

    sprintf(tmp,"extream size (%i x %i)", int(100*setting.win_w), int(100*setting.win_h));
    autoshot->addItem(new CED_SubSubMenu(tmp,AUTOSHOT_IMAGE100));

    CED_SubMenu *tools=new CED_SubMenu("Tools");
    tools->addItem(screenshot);
    tools->addItem(new CED_SubSubMenu("---",0));
    if(setting.fps){
        tools->addItem(new CED_SubSubMenu("[X] Show FPS",FPS));
    }else{
        tools->addItem(new CED_SubSubMenu("[ ] Show FPS",FPS));
    }
    tools->addItem(new CED_SubSubMenu("---",0));
    if(setting.autoshot) {
        tools->addItem(new CED_SubSubMenu("[X] Auto shot",AUTOSHOT));
    }
    else {
        tools->addItem(new CED_SubSubMenu("[ ] Auto shot",AUTOSHOT));
    }
    tools->addItem(autoshot);
    ced_menu->addSubMenu(tools);


    CED_SubMenu *help=new CED_SubMenu("Help");
    if(showHelp){
        help->addItem(new CED_SubSubMenu("[X] Show keyboard shortcuts",HELP));
    }else{
        help->addItem(new CED_SubSubMenu("[ ] Show keyboard shortcuts",HELP));
    }
    help->addItem(new CED_SubSubMenu("---",0));
    help->addItem(new CED_SubSubMenu("Contact CED team (hauke.hoelbe@desy.de)",0));
    ced_menu->addSubMenu(help);
}
