#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <ced.h>

#include "screenshot.h"
#include "ui/input.h"
#include "render.h"

using namespace std;

// Owned by glced.cc.
extern float WORLD_SIZE;

static int numpict=0;

int save_pixmap_as_tga(unsigned char *buffer_all,const char *name,int wi, int hi){
    //based on: http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=44286

    int header_size=24;
    int mem_size = wi*hi*3;
    //unsigned char tmp;
    FILE *out_file;
    unsigned char *header;

    if (!(header = (unsigned char *) calloc(1, header_size))) { return(-1); }

    //write header
    header[2] = 2;  // uncompressed
    header[12] = wi & 255;
    header[13] = wi >> 8;
    header[14] = hi & 255;
    header[15] = hi >> 8;
    header[16] = 24;    // 24 bits per pix

    if (!(out_file = fopen(name, "wb"))) { return(-2); }

    fwrite(header, sizeof(unsigned char), header_size, out_file);
    fwrite(buffer_all, sizeof(unsigned char), mem_size, out_file);

    fclose(out_file);
    return(0);
}

void screenshot(const char *, int times)
{
    if(times > 100){
        std::cout << "Sorry 100x100 are the max value" << std::endl ;
        return;
    }

    setting.screenshot_sections=times;


    //int HEADER_SIZE=24;
    unsigned char *buffer_all;
    unsigned char *buffer[100*100];

    //char filename[100];

    int w=(int)window_width;
    int h=(int)window_height;

    int buf_size = (w*h*3);

    std::cout << "Generating screenshot (" << w*times << "x" << h*times << "):" << std::endl;

    //int buf_size_all = HEADER_SIZE + w*h*3 *times*times;

    int buf_size_all = w*h*3 *times*times;


    std::cout << "    Requesting memory ";
    for(int i=0;i<times*times;i++){
        std::cout << ".";
        std::cout.flush();
        if (!(buffer[i] = (unsigned char *) calloc(1, buf_size)))
        {
            return;
        }
    }
    if (!(buffer_all = (unsigned char *) calloc(1, buf_size_all)))
    {
        return;
    }
    std::cout << " Done" << std::endl;


    std::cout << "    Generating image ";

    if(setting.persp == true){
        glTranslatef(0.0, 0.0, +2000); //HOTFIX!!! TODO: find the place where this translation is made
        double near_plane=200.;
        for(int i=0;i<times;i++){
            for(int j=0;j<times;j++){

                std::cout << ".";
                std::cout.flush();

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                glViewport(0,0,w,h);

                if(w >= h){
                glFrustum((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*w*1.0/h,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*w*1.0/h,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times),
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times),
                          near_plane ,50000.0*mm.sf*2+50000/(mm.sf*2));
                }else{
                glFrustum((-1*near_plane/2.      + 2*i*(near_plane/2.)/times),
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times),
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*h*1.0/w,
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*h*1.0/w,
                          near_plane*h*1./w ,50000.0*mm.sf*2+50000/(mm.sf*2));

                }

                glViewport(0,0,w,h);
                glMultMatrixf(glm::value_ptr(glm::lookAt(
                    glm::vec3(0,0,2000),
                    glm::vec3(0,0,0),
                    glm::vec3(0,1,0)
                )));
                glViewport(0,0,w,h);

                glMatrixMode(GL_MODELVIEW);
                write_world_into_front_buffer();

                glViewport(0,0,w,h);

                glPixelStorei( GL_PACK_ALIGNMENT, 1 );
                glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer[i+j*times]);

                glMatrixMode(GL_MODELVIEW);
            }
        }
     }else{
        double near_plane=200./(WORLD_SIZE/10);

        for(int i=0;i<times;i++){
            for(int j=0;j<times;j++){

                std::cout << ".";
                std::cout.flush();

                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();

                glViewport(0,0,w,h);

                if(w >= h){


                glOrtho((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*w*1.0/h*WORLD_SIZE,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*w*1.0/h*WORLD_SIZE,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*WORLD_SIZE,
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*WORLD_SIZE,
                          near_plane ,(50000.0*mm.sf*2+50000/(mm.sf*2)));
                }else{

                //near_plane*=1./(h*1./w);
                double tmp2=WORLD_SIZE;
                WORLD_SIZE*=w*1./h;
                glOrtho((-1*near_plane/2.      + 2*i*(near_plane/2.)/times)*WORLD_SIZE,
                          (-1*near_plane/2.      +(i+1)*2*(near_plane/2.)/times)*WORLD_SIZE,
                          (-1*(near_plane/2.)    + 2*j*(near_plane/2.)/times)*h*1.0/w*WORLD_SIZE,
                          (-1*(near_plane/2.)    +(j+1)*2*(near_plane/2.)/times)*h*1.0/w*WORLD_SIZE,
                          near_plane,(50000.0*mm.sf*2+50000/(mm.sf*2)));

                WORLD_SIZE=tmp2;

                }
                glViewport(0,0,w,h);
                glMultMatrixf(glm::value_ptr(glm::lookAt(
                    glm::vec3(0,0,2000),
                    glm::vec3(0,0,0),
                    glm::vec3(0,1,0)
                )));
                glViewport(0,0,w,h);
                glMatrixMode(GL_MODELVIEW);
                write_world_into_front_buffer();

                glViewport(0,0,w,h);

                glPixelStorei( GL_PACK_ALIGNMENT, 1 );
                glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer[i+j*times]);

                glMatrixMode(GL_MODELVIEW);
            }
        }
    }
    std::cout << " Done" << std::endl;

    for(int k=0;k<times;k++){
        for(int l=0;l<h;l++){
            for(int j=0;j<times;j++){
                for(int i=0; i < w*3; i++){
                    buffer_all[i+w*3*j+l*w*3*times+k*times*w*h*3]=buffer[j+times*k][i+l*w*3];
                }
            }
        }
    }

    //RGB -> BGR
    char tmp;
    for(int j=0;j<buf_size_all;j+=3){
        tmp = buffer_all[j];
        buffer_all[j] = buffer_all[j+2];
        buffer_all[j+2] = tmp;
    }

    numpict++;
    std::ostringstream foutname;
    char *imgfile;
    imgfile = getenv ( "CED_IMAGEFILE" );
    if(imgfile != NULL){
      foutname << imgfile << "-" << numpict << ".tga" << std::ends;
    }else{
      foutname << "glced-" << numpict << ".tga" << std::ends;
    }
    std::string cfout=foutname.str();
    int spret=save_pixmap_as_tga(buffer_all, cfout.c_str(), w*times, h*times);
    if ( spret == 0 ) {
      std::cout << "Imagefile was written in " << cfout << std::endl ;
    }
    else {
      std::cout << "Failed to write file to " << cfout << std::endl;
    }

    std::cout << "    Clean memory ";
    for(int i=0;i<times*times;i++){
        std::cout << ".";
        std::cout.flush();
        free(buffer[i]);
    }
    free(buffer_all);
    std::cout << " Done" << std::endl;

    setting.screenshot_sections=1;
}
