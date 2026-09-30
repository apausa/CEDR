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

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>

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

#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>


#include <ced_menu.h>
#include "io/screenshot.h"
#include "ui/menu.h"
#include "render.h"
#include "cli.h"
#include "utils/helpers.h"

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
float BG_COLOR[4];
extern int SELECTED_ID ;

//fg - make axe a global to be able to rescale the world volume
GLfloat axe[][3]={
  { 0., 0., 0., },
  { DEFAULT_WORLD_SIZE/2, 0., 0. },
  { 0., DEFAULT_WORLD_SIZE/2, 0. },
  { 0., 0., DEFAULT_WORLD_SIZE/2 }
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

Point pick_point;
Point pre_pick_point;
int selected_layer;
bool  select_nothing=true;



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



    parseCliArgs(argc, argv);

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
