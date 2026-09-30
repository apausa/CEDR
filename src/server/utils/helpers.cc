#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <ced.h>
#include <ced_cli.h>

#include "utils/helpers.h"

extern CEDsettings setting;
extern float WORLD_SIZE;
extern GLfloat axe[][3];
extern float BG_COLOR[4];
extern int showHelp;
extern bool ced_needs_redraw;

// allows to reset the visible world size
void set_world_size( float length) {
  WORLD_SIZE = length ;
  axe[1][0] = WORLD_SIZE / 2. ;
  axe[2][1] = WORLD_SIZE / 2. ;
  axe[3][2] = WORLD_SIZE / 2. ;
};

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

void toggleHelpWindow(void){ //hauke
    if(showHelp == 1){
        showHelp=0;
    }else{
        showHelp=1;
    }
    ced_needs_redraw = true;
}
