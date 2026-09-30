#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <math.h>
#include <vector>

#include "cylinder_geometry.h"

#define PI 3.14159265358979323846f 

// Draw a partial cylinder made of lines (for the detector geometry)
void drawPartialLineCylinder(
    double length,
    double R /*radius*/,
    double iR /*inner radius*/,
    int edges,
    double angle_cut_off,
    double angle_cut_off_left,
    bool outer_face,
    bool inner_face
){
    //return;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH,GL_NICEST);
    glDepthMask(GL_FALSE);
    double phi, x, xl; 
    int i,j;
    phi=360.0/edges;
    x=0;

    glPushMatrix(); //save matrix on the stack

    glTranslatef(0, 0, length/2);

    //draw the two ends
    for(j=0;j<2;j++){
        
        if(j==0){glTranslatef(0, 0, -length/2);}
        else if(j==1){glTranslatef(0, 0, length);}

        phi=(360.0/edges);

        i=int(angle_cut_off_left/phi+1);  
        phi=(360.0/edges)*(i+edges);
        xl = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)+angle_cut_off_left)*2*PI/360.0);

        if(inner_face){
            glBegin(GL_LINE);
            glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));
            glVertex2d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0));
            glEnd();

        }
        
        if(outer_face){
            glBegin(GL_LINE);
            glVertex2d(R*xl*sin((angle_cut_off_left)*2*PI/360.0),R*xl*cos(angle_cut_off_left*2*PI/360.0));
            glVertex2d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));
            glEnd();
        }

        glBegin(GL_LINE_STRIP );
        glVertex2d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));

        i=i+1; 


        for(;i<edges+1;i++){
            phi=360.0/edges*i;
            if(360.0-phi <= angle_cut_off){
                x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);

                if(outer_face){
                    glVertex2d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                    glVertex2d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                }
                break;
            }else{
                if(outer_face){
                    glVertex2d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                    if(i != 0){
                        glVertex2d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                    }
                }
            }
        }
        glEnd();

        i = int(angle_cut_off_left/(360.0/edges)+1);
        //i=(int) angle_cut_off_left/phi+1;  
        i=i+1; 

        //glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));

        glBegin(GL_LINE_STRIP);

    /*
        if(inner_face){
            x = cos(2*PI/edges/2.0)/cos((360.0- (phi-360.0/edges/2.0)-angle_cut_off_left)*2.0*PI/360.0);
            glVertex2d(iR*x*sin((360.0-angle_cut_off_left)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off_left)*2*PI/360.0));

        }
    */

        for(;i<edges+1;i++){
            phi=360.0/edges*i;

            if(360.0-phi <= angle_cut_off){
                x = cos(2*PI/edges/2.0)/cos((360.0- (phi-360.0/edges/2.0)-angle_cut_off)*2.0*PI/360.0);

                if(inner_face){
                    glVertex2d(iR*sin(360.0/edges*(i-1)*2.0*PI/360.0), iR*cos(360.0/edges*(i-1)*2.0*PI/360.0));
                    glVertex2d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                } 
                break; 
            }else{
                // //glVertex2d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                if(inner_face){
                    if(i != 0){
                        glVertex2d(iR*sin(360.0/edges*(i-1)*2.0*PI/360.0), iR*cos(360.0/edges*(i-1)*2.0*PI/360.0));
                        glVertex2d(iR*sin(phi*2.0*PI/360.0), iR*cos(phi*2.0*PI/360.0));
                    }
                }
            }
        }
        glEnd();

    phi=(360.0/edges);
    i=int(angle_cut_off_left/phi+1);  

    //outer
    if(outer_face){
        glBegin(GL_LINES);
        glVertex2d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0));
        glVertex2d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));
        glEnd();

    }


    if(inner_face){
        if(iR > 0){
            //inner
            glBegin(GL_LINES);
            glVertex2d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0));
            glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));
            glEnd();
        }
    }


    }

    glTranslatef(0, 0, -length/2);

    //close 2 to cuts, if cutting
    if(angle_cut_off > 0.0 || angle_cut_off_left > 0.0){
        if(outer_face){
            glBegin(GL_LINES);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glEnd();
        }

        if(inner_face){
            glBegin(GL_LINES);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glEnd();
        }

        if(outer_face){
            glBegin(GL_LINES);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glEnd();
       
            glBegin(GL_LINES);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glEnd();
        }

        if(inner_face){
            glBegin(GL_LINES);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);

            glBegin(GL_LINES);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glEnd();

            glBegin(GL_LINES);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glEnd();
        }
    }


    //draw the cylinder
    if(edges < 20){ //only draw the parallel to z-axes lines if there are not to much
        phi=(360.0/edges);
        i=int( angle_cut_off_left/phi+1);  

        //outer
        if(outer_face){
            glBegin(GL_LINE_LOOP);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), length/2);
            glVertex3d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), -length/2);
            glEnd();
        }


        if(inner_face){
            if(iR > 0){
                //inner
                glBegin(GL_LINE_LOOP);
                glVertex3d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0), -length/2);
                glVertex3d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0), length/2);
                glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
                glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
                glEnd();
            }
        }


        i=int(angle_cut_off_left/phi+2);  


        for(;i<edges+1;i++){
            phi=360.0/edges*i;
            double phi2=360.0/edges*(i-1);

            if(360.0-phi <= angle_cut_off){
                double xx = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);

                if(outer_face){
                    //outer
                    glBegin(GL_LINE_LOOP);
                    glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),-length/2);
                    glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),length/2);
                    glVertex3d(R*xx*sin((360.0-angle_cut_off)*2*PI/360.0), R*xx*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
                    glVertex3d(R*xx*sin((360.0-angle_cut_off)*2*PI/360.0), R*xx*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
                    glEnd();
                }

                if(iR > 0){
                    if(inner_face){
                        //inner:
                        glBegin(GL_LINE_LOOP);
                        glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2);
                        glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2);
                        glVertex3d(iR*xx*sin((360.0-angle_cut_off)*2*PI/360.0), iR*xx*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
                        glVertex3d(iR*xx*sin((360.0-angle_cut_off)*2*PI/360.0), iR*xx*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
                        glEnd();
                    }
                }
                break;
            }else{
                if(outer_face){
                    //outer
                    glBegin(GL_LINE_LOOP);
                    glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),-length/2);
                    glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),length/2);
                    glVertex3d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0), length/2);
                    glVertex3d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0), -length/2);
                    glEnd();
                }

                if(iR > 0){
                    if(inner_face){
                        //inner
                        glBegin(GL_LINE_LOOP);
                        glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2);
                        glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2);
                        glVertex3d(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), length/2);
                        glVertex3d(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), -length/2);
                        glEnd();
                    }
                }
            }
        }
    }
    glPopMatrix(); //get the saved matrix back

    glDepthMask(GL_TRUE);
    glDisable(GL_LINE_SMOOTH);

    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

}

struct my_point {
    // Constructor from two doubles which is necessary for c++17
    constexpr my_point(double xx, double yy) : x(xx), y(yy) {}

    double x;
    double y;
};

struct T {double a; int b;};

struct point3d{
    double x;
    double y;
    double z;
};

void calNormals(point3d &n, point3d p1_, point3d p2_, point3d p3_){
    point3d p2={p2_.x-p1_.x, p2_.y-p1_.y, p2_.z-p1_.z};
    point3d p3={p3_.x-p1_.x, p3_.y-p1_.y, p3_.z-p1_.z};

    //calulate crossproduct
    n.x=(p2.y*p3.z - p3.y*p2.z);
    n.y=(p2.z*p3.x - p3.z*p2.x);
    n.z=(p2.x*p3.y - p3.x*p2.y);
    
    double factor=1.0/pow(pow(n.x,2)+pow(n.y,2)+pow(n.z,2),0.5);
    n.x=factor*n.x;
    n.y=factor*n.y;
    n.z=factor*n.z;
}

// Draw a partial cylinder made of planes (for the detector geometry)
void drawPartialCylinder(
    double length,
    double R /*radius*/,
    double iR /*inner radius*/,
    int edges,
    double angle_cut_off,
    double angle_cut_off_left,
    bool outer_face,
    bool inner_face,
    double irotate
){
    //glDisable(GL_BLEND);
    //return;
    using namespace std;
    // cout << "inside drawPartialCylinder: outer" << outer_face << " inner: " << inner_face << endl;

    static double edge_vec[12];
    static vector<my_point> inner_side_points;

    double phi=360.0/edges;
    int i,j;
    double x, xl;
    
    glPushMatrix(); //save the old matrix on the stack

    i=int(angle_cut_off_left/phi+1);  
    phi=(360.0/edges)*(i+edges);
    xl = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)+angle_cut_off_left)*2*PI/360.0);
    x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);

    glTranslatef(0, 0, length/2);


    //draw the cylinder
    phi=(360.0/edges);
    i=int( angle_cut_off_left/phi+1);  

    if(inner_face == true){
        //inner
        //struct point3d p1={1,2,3};

        struct point3d p1 = {iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0), -length/2};
        struct point3d p2 = {iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0), length/2};
        struct point3d p3 = {iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2};
        struct point3d p4 = {iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2};
        struct point3d n;
        calNormals(n,p1,p2,p3);

        glBegin(GL_QUADS);
        glVertex3d(p1.x,p1.y,p1.z);
        glNormal3f(n.x,n.y,n.z);
        glVertex3d(p2.x,p2.y,p2.z);
        glVertex3d(p3.x,p3.y,p3.z);
        glVertex3d(p4.x,p4.y,p4.z);
        glEnd();
    }


    i=int( angle_cut_off_left/phi+2);  
    for(;i<edges+1;i++){
        phi=360.0/edges*i;
        double phi2=360.0/edges*(i-1);

        if(360.0-phi <= angle_cut_off){
            x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);


            if(inner_face == true){
                //inner:
                struct point3d p1 = {iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2};
                struct point3d p2 = {iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2};
                struct point3d p3 = {iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2};
                struct point3d p4 = {iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2};
                struct point3d n;
                calNormals(n,p1,p2,p3);

                glBegin(GL_QUADS);
                glVertex3d(p1.x,p1.y,p1.z);
                glNormal3f(n.x,n.y,n.z);
                glVertex3d(p2.x,p2.y,p2.z);
                glVertex3d(p3.x,p3.y,p3.z);
                glVertex3d(p4.x,p4.y,p4.z);
                glEnd();

                //glBegin(GL_QUADS);
                //glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2);
                //glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2);
                //glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
                //glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
                //glEnd();
            }
            break;
        }else{
            if(inner_face == true){
                //inner
                struct point3d p1 = {iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2};
                struct point3d p2 = {iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2};
                struct point3d p3 = {iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), length/2};
                struct point3d p4 = {iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), -length/2};
                struct point3d n;
                calNormals(n,p1,p2,p3);

                glBegin(GL_QUADS);
                glVertex3d(p1.x,p1.y,p1.z);
                glNormal3f(n.x,n.y,n.z);
                glVertex3d(p2.x,p2.y,p2.z);
                glVertex3d(p3.x,p3.y,p3.z);
                glVertex3d(p4.x,p4.y,p4.z);
                glEnd();


                //glBegin(GL_QUADS);
                //glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),-length/2);
                //glVertex3d(iR*sin(phi2*2*PI/360.0), iR*cos(phi2*2*PI/360.0),length/2);
                //glVertex3d(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), length/2);
                //glVertex3d(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0), -length/2);
                //glEnd();
            }
        }
    }



    phi=(360.0/edges);
    i=int(angle_cut_off_left/phi+1);  

    if(outer_face == true){
        //outer
        struct point3d p1 = {R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2};
        struct point3d p2 = {R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2};
        struct point3d p3 = { R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), length/2};
        struct point3d p4 = {       R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), -length/2};
        struct point3d n;    
        calNormals(n,p1,p2,p3);

        glBegin(GL_QUADS);
        glVertex3d(p1.x,p1.y,p1.z);
        glNormal3f(n.x,n.y,n.z);
        glVertex3d(p2.x,p2.y,p2.z);
        glVertex3d(p3.x,p3.y,p3.z);
        glVertex3d(p4.x,p4.y,p4.z);
        glEnd();

        //glBegin(GL_QUADS);
        //glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
        //glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);

        //glVertex3d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), length/2);
        //glVertex3d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0), -length/2);
        //glEnd();
    }


    i=int( angle_cut_off_left/phi+2);  

    for(;i<edges+1;i++){
        phi=360.0/edges*i;
        double phi2=360.0/edges*(i-1);

        if(360.0-phi <= angle_cut_off){
            double xx = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);

            if(outer_face == true){
                //outer

                struct point3d p1 = {R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),-length/2};
                struct point3d p2 = {R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),length/2};
                struct point3d p3 = {R*xx*sin((360.0-angle_cut_off)*2*PI/360.0), R*xx*cos((360.0-angle_cut_off)*2*PI/360.0), length/2};
                struct point3d p4 = {R*xx*sin((360.0-angle_cut_off)*2*PI/360.0), R*xx*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2};
                struct point3d n;    
                calNormals(n,p1,p2,p3);

                glBegin(GL_QUADS);
                glVertex3d(p1.x,p1.y,p1.z);
                glNormal3f(n.x,n.y,n.z);
                glVertex3d(p2.x,p2.y,p2.z);
                glVertex3d(p3.x,p3.y,p3.z);
                glVertex3d(p4.x,p4.y,p4.z);
                glEnd();

                //glBegin(GL_QUADS);
                //glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),-length/2);
                //glVertex3d(R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),length/2);
                //glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
                //glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
                //glEnd();
            }

                break;
        }else{
            if(outer_face == true){

                //outer
                struct point3d p1 = {R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),-length/2};
                struct point3d p2 = {R*sin(phi2*2*PI/360.0), R*cos(phi2*2*PI/360.0),length/2};
                struct point3d p3 = {R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0), length/2};
                struct point3d p4 = {R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0), -length/2};

                struct point3d n;
                calNormals(n,p1,p2,p3);

                glBegin(GL_QUADS);

                glVertex3d(p1.x,p1.y,p1.z);
                glNormal3f(n.x,n.y,n.z);
                glVertex3d(p2.x,p2.y,p2.z);
                glVertex3d(p3.x,p3.y,p3.z);
                glVertex3d(p4.x,p4.y,p4.z);

                glEnd();

            }

        }
    }

    //close 2 to cuts, if cutting
    if(angle_cut_off > 0.0 || angle_cut_off_left > 0.0){
        if(inner_face == false){
                struct point3d p1 = {R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2};
                struct point3d p2 = {R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2};
                struct point3d p3 = {edge_vec[0]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[1],  edge_vec[0]*sin(irotate/360*2*PI) + edge_vec[1]*cos(irotate/360.0*2*PI), edge_vec[2]};
                struct point3d p4 = {edge_vec[3]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[4],  edge_vec[3]*sin(irotate/360*2*PI) + edge_vec[4]*cos(irotate/360.0*2*PI), edge_vec[5]};
                struct point3d n;    
                calNormals(n,p1,p2,p3);

                glBegin(GL_QUADS);
                glVertex3d(p1.x,p1.y,p1.z);
                glNormal3f(n.x,n.y,n.z);
                glVertex3d(p2.x,p2.y,p2.z);
                glVertex3d(p3.x,p3.y,p3.z);
                glVertex3d(p4.x,p4.y,p4.z);
                glEnd();


            //glBegin(GL_QUADS);
            //glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            //glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            //glVertex3d(edge_vec[0]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[1],  edge_vec[0]*sin(irotate/360*2*PI) + edge_vec[1]*cos(irotate/360.0*2*PI), edge_vec[2]);
            //glVertex3d(edge_vec[3]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[4],  edge_vec[3]*sin(irotate/360*2*PI) + edge_vec[4]*cos(irotate/360.0*2*PI), edge_vec[5]);
            //glEnd();

           //struct point3d  p1 = {R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2};
           //struct point3d  p2 = {R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2};
           //struct point3d  p3 = {edge_vec[6]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[7],  edge_vec[6]*sin(irotate/360*2*PI) + edge_vec[7]*cos(irotate/360.0*2*PI), edge_vec[8]};
           //struct point3d  p4 = {edge_vec[9]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[10],  edge_vec[9]*sin(irotate/360*2*PI) + edge_vec[10]*cos(irotate/360.0*2*PI), edge_vec[11]};
           //struct point3d n;
           // calNormals(n,p1,p2,p3);

           // glBegin(GL_QUADS);
           // glVertex3d(p1.x,p1.y,p1.z);
           // glNormal3f(n.x,n.y,n.z);
           // glVertex3d(p2.x,p2.y,p2.z);
           // glVertex3d(p3.x,p3.y,p3.z);
           // glVertex3d(p4.x,p4.y,p4.z);
           // glEnd();


            glBegin(GL_QUADS);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(edge_vec[6]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[7],  edge_vec[6]*sin(irotate/360*2*PI) + edge_vec[7]*cos(irotate/360.0*2*PI), edge_vec[8]);
            glVertex3d(edge_vec[9]*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*edge_vec[10],  edge_vec[9]*sin(irotate/360*2*PI) + edge_vec[10]*cos(irotate/360.0*2*PI), edge_vec[11]);
            glEnd();
 
        }else if(outer_face == false){
 

            edge_vec[0] = iR*x*sin((360.0-angle_cut_off)*2*PI/360.0);
            edge_vec[1] = iR*x*cos((360.0-angle_cut_off)*2*PI/360.0);
            edge_vec[2] = length/2;

            edge_vec[3] = iR*x*sin((360.0-angle_cut_off)*2*PI/360.0);
            edge_vec[4] = iR*x*cos((360.0-angle_cut_off)*2*PI/360.0);
            edge_vec[5] = -length/2;

            edge_vec[6] = iR*xl*sin((angle_cut_off_left)*2*PI/360.0);
            edge_vec[7] = iR*xl*cos((angle_cut_off_left)*2*PI/360.0);
            edge_vec[8] = length/2;

            edge_vec[9] = iR*xl*sin((angle_cut_off_left)*2*PI/360.0);
            edge_vec[10] = iR*xl*cos((angle_cut_off_left)*2*PI/360.0);
            edge_vec[11] = -length/2;

        }else{
            ;
            glBegin(GL_QUADS);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glVertex3d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), length/2);
            glVertex3d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0), -length/2);
            glEnd();

            glBegin(GL_QUADS);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glVertex3d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), length/2);
            glVertex3d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0), -length/2);
            glEnd();
      }
    }

    //draw the two ends 
    //run this block when drawing the hole cylinder
    if(outer_face == true && inner_face == true){
        for(j=0;j<2;j++){
            if(j==0){glTranslatef(0, 0, -length/2);}
            else if(j==1){glTranslatef(0, 0, length);}

            phi=(360.0/edges);

            i=int( angle_cut_off_left/phi+1);  
            phi=(360.0/edges)*(i+edges);
            xl = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)+angle_cut_off_left)*2*PI/360.0);



            glBegin(GL_TRIANGLES);
            glVertex2d(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos((angle_cut_off_left)*2*PI/360.0));
            glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));
            glVertex2d(iR*xl*sin((angle_cut_off_left)*2*PI/360.0),iR*xl*cos((angle_cut_off_left)*2*PI/360.0));
            glEnd();
            
            

            glBegin(GL_TRIANGLES);
            glVertex2d(R*xl*sin((angle_cut_off_left)*2*PI/360.0),R*xl*cos(angle_cut_off_left*2*PI/360.0));
            glVertex2d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));
            glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));
            glEnd();

            //phi=(360.0/edges)*i;
            //i=(int) angle_cut_off_left/phi+1;  

            glBegin(GL_TRIANGLE_STRIP );
            glVertex2d(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));
            glVertex2d(iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0));

            i=i+1; 


            for(;i<edges+1;i++){
                phi=360.0/edges*i;
                if(360.0-phi <= angle_cut_off){
                    x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);
                    glVertex2d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                    glVertex2d(iR*sin(360.0/edges*(i-1)*2*PI/360.0), iR*cos(360.0/edges*(i-1)*2*PI/360.0));
                    glVertex2d(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                    glVertex2d(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                    break;
                }else{
                    glVertex2d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                    if(i != 0){
                        glVertex2d(iR*sin(360.0/edges*(i-1)*2*PI/360.0), iR*cos(360.0/edges*(i-1)*2*PI/360.0));
                        glVertex2d(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0));
                        glVertex2d(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                    }
                }
            }
            glEnd();
        }

        glTranslatef(0, 0, -length/2);
        //glPopMatrix(); //restore the matrix from the stack
        //glEnable(GL_BLEND);
    }else if(outer_face == false && inner_face==true){ //fill up vectors to draw the detector in next step

        //glEnd();
        //inner_side_points.clear();
       // cout << "inner side size" << inner_side_points.size() << endl;


        for(j=0;j<2;j++){
            if(j==0){glTranslatef(0, 0, -length/2);}
            else if(j==1){glTranslatef(0, 0, length); }
        }

            phi=(360.0/edges);

            i=int( angle_cut_off_left/phi+1);  
            phi=(360.0/edges)*(i+edges);
            xl = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)+angle_cut_off_left)*2*PI/360.0);




            //inner_side_points.push_back((my_point){iR*sin(360.0/edges*(i)*2*PI/360.0),iR*cos(360.0/edges*(i)*2*PI/360.0)});
            inner_side_points.emplace_back(iR*xl*sin((angle_cut_off_left)*2*PI/360.0), iR*xl*cos((angle_cut_off_left)*2*PI/360.0));

            i=i+1; 


            for(;i<edges+1;i++){
                phi=360.0/edges*i;
                if(360.0-phi <= angle_cut_off){
                    x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);
                    inner_side_points.emplace_back(iR*sin(360.0/edges*(i-1)*2*PI/360.0), iR*cos(360.0/edges*(i-1)*2*PI/360.0));
                    inner_side_points.emplace_back(iR*x*sin((360.0-angle_cut_off)*2*PI/360.0), iR*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                    break;
                }else{
                    if(i != 0){
                        inner_side_points.emplace_back(iR*sin(360.0/edges*(i-1)*2*PI/360.0), iR*cos(360.0/edges*(i-1)*2*PI/360.0));
                        inner_side_points.emplace_back(iR*sin(phi*2*PI/360.0), iR*cos(phi*2*PI/360.0));
                    }
                }
            }
             //glEnd();
        //}

        //glPopMatrix(); //restore the matrix from the stack

        
    }else if(inner_face == false && outer_face == true){
        vector<my_point> outer_side_points;
        //outer_side_points.clear();
        phi=(360.0/edges);

        i=int( angle_cut_off_left/phi+1);  
        phi=(360.0/edges)*(i+edges);
        xl = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)+angle_cut_off_left)*2*PI/360.0);

    //        outer_side_points.push_back((my_point){});



        
        outer_side_points.emplace_back(R*xl*sin((angle_cut_off_left)*2*PI/360.0), R*xl*cos(angle_cut_off_left*2*PI/360.0));
        outer_side_points.emplace_back(R*sin(360.0/edges*(i)*2*PI/360.0),R*cos(360.0/edges*(i)*2*PI/360.0));

        i=i+1; 


        for(;i<edges+1;i++){
            phi=360.0/edges*i;
            if(360.0-phi <= angle_cut_off){
                x = cos(2*PI/edges/2)/cos((360.0- (phi-360.0/edges/2)-angle_cut_off)*2*PI/360.0);

                outer_side_points.emplace_back(R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0));
                //outer_side_points.push_back((my_point){R*x*sin((360.0-angle_cut_off)*2*PI/360.0), R*x*cos((360.0-angle_cut_off)*2*PI/360.0)});
                break;
            }else{
                outer_side_points.emplace_back(R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0));
                //if(i != 0){
                //    outer_side_points.push_back((my_point){R*sin(phi*2*PI/360.0), R*cos(phi*2*PI/360.0)});
                //}
            }
        }

        for(unsigned int ii=0;ii<inner_side_points.size();ii++){ //transform the points to inner rotate angle
            my_point tmp = my_point(inner_side_points[ii]);
            inner_side_points[ii].x=tmp.x*cos(irotate/360*2*PI) - sin(irotate/360*2*PI)*tmp.y;
            inner_side_points[ii].y=tmp.x*sin(irotate/360*2*PI) + tmp.y*cos(irotate/360.0*2*PI);
        }

        for(j=0;j<2;j++){  //here do the drawing
            //glEnd();
            if(j==0){glTranslatef(0, 0, -length/2);}
            else if(j==1){glTranslatef(0, 0, length);}


            //glBegin(GL_TRIANGLE_STRIP );


            unsigned int inner_c,outer_c;
            inner_c=0;outer_c=0;


            //glBegin(GL_LINE_STRIP);

            glBegin(GL_TRIANGLE_STRIP);

            glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
            glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);

            if(inner_side_points.size() < outer_side_points.size()){ //outer cylinder have more edges
                int step=(int)outer_side_points.size()/inner_side_points.size();

            
                while(inner_c < inner_side_points.size()-1 && outer_c + step < outer_side_points.size()-1){
                    for(int k=0;k<step;k++){
                        outer_c++;
                        glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
                        glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);
                    }
                    inner_c++;
                }
            
            }else{ //inner cylinder have more edges
                int step=(int)inner_side_points.size()/outer_side_points.size();
            

                //glBegin(GL_TRIANGLE_STRIP);
                //glBegin(GL_LINE_STRIP);
                while(outer_c < outer_side_points.size()-1 && inner_c + step < inner_side_points.size()-1){
                    for(int k=0;k<step;k++){
                        //cout << "max inner: " << inner_side_points.size()-1 << "outer max: " << outer_side_points.size()-1 << endl;
                        //cout << "inner counter: " << inner_c << " ( " << inner_side_points[inner_c].x << ", " 
                        //      << outer_side_points[outer_c].y << ") outer counter: " << outer_c << endl;
                        glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);
                        glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
                        inner_c++;
                    }
                    outer_c++;
                }

                
            }

            inner_c--;
            outer_c--;
            for(;outer_c < outer_side_points.size()-1; outer_c++){
                glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);
                glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
            }

            for(;inner_c < inner_side_points.size()-1; inner_c++){
                glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);
                glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
            }

            glVertex2d(inner_side_points[inner_c].x, inner_side_points[inner_c].y);
            glVertex2d(outer_side_points[outer_c].x, outer_side_points[outer_c].y);

            glEnd();

    //            glBegin(GL_LINE_STRIP);
    //            for(int i=0;i<inner_side_points.size() && i<outer_side_points.size();i++){
    //
    //                glVertex2d(inner_side_points[i].x, inner_side_points[i].y);
    //                glVertex2d(outer_side_points[i].x, outer_side_points[i].y);
    //            }
    //
    //            glEnd();

            

            

        }
        inner_side_points.clear();
        outer_side_points.clear();
        glTranslatef(0, 0, -length/2);
        //glPopMatrix(); //restore the matrix from the stack
        //glEnable(GL_BLEND);
    }

                

    glPopMatrix(); 

}
