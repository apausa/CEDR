#include "render.h"

/*
 * Helper function
 */

void ced_color(unsigned rgba){
    glColor4ub((rgba>>16)&0xff,(rgba>>8)&0xff,(rgba)&0xff, 0xff-((rgba>>24)&0xff));
}
