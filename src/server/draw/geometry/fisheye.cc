#include "fisheye.h"

extern double fisheye_alpha;

CED_Point fisheye_transform(const float x, const float y, const float z, const double scale_factor) {
    CED_Point p_final;
    if(scale_factor < 1e-10) {
        //If fisheye_alpha < observable value, do nothing
        p_final.x = x;
        p_final.y = y;
        p_final.z = z;
    }
    else {
        float rho = sqrt(x*x + y*y);
        rho = rho/(1.0+scale_factor*rho);
        float r = sqrt(rho*rho+z*z);
        if(r==0){r=0.0000000000001; } //hauke: in some cases r = 0, not good for next line (z/r)
        float cos_theta = z/r;
        float theta = acos(cos_theta);
        float phi = atan2(y,x);
        p_final.x = r*cos(phi)*sin(theta);
        p_final.y = r*sin(phi)*sin(theta);
        //p_final.z = z/(1.0 + fisheye_alpha*abs(z)); //hauke: missing 'f' in abs???
        p_final.z = z/(1.0 + fisheye_alpha*fabs(z));

    }

    return p_final;
}
