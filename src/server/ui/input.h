#pragma once

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

enum {
    MOUSE_DOWN = 0,
    MOUSE_UP = 1
};
enum {
    MOUSE_LEFT = 0,
    MOUSE_MIDDLE = 1,
    MOUSE_RIGHT = 2
};
enum {
    KEY_LEFT = 100,
    KEY_UP = 101,
    KEY_RIGHT = 102,
    KEY_DOWN = 103,
    KEY_PAGE_UP = 104,
    KEY_PAGE_DOWN = 105,
    KEY_HOME = 106,
    KEY_END = 107,
    KEY_INSERT = 108
};

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} Point;

struct CameraState {
    GLfloat va; // vertical angle
    GLfloat ha; // horisontal angle
    GLfloat sf; // scale factor
    Point mv; // the center
    GLfloat va_start;
    GLfloat ha_start;
    GLfloat sf_start;
    Point mv_start;
};

extern CameraState mm;
extern CameraState mm_reset;

void mouse(int btn, int state, int x, int y);
void mouseWheel(int, int dir, int, int);
void keypressed(unsigned char key, int x, int y);
void SpecialKey(int key, int, int);
void motion(int x, int y);
