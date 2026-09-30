#pragma once

#include <math.h>
#include <ced_cli.h>

//SM-H: Takes a given point, and returns the fisheye transformed version. Based on transform given in
//'Event display: Can We See What We Want to See',
//H. Drevermann, D. Kuhn, B.S. Nilsson, 1995
//TODO: More elegant (and eficient) implementation possible.
//See http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=262910
CED_Point fisheye_transform(const float x, const float y, const float z, const double scale_factor);

//SM-H: The same as above, but just applied to r or z rather than a whole cartesian co-ordinate system
//CED co-ordinates only defined up to float precision
inline float single_fisheye_transform(float c, const double scale_factor) {
    return c/(1.0+scale_factor*fabs(c));
}
