#include <SDL3/SDL.h>

#include "render.h"
#include "../geometry/fisheye.h"
#include "../ui/selection.h"

/*
 * Hit element
 */

void ced_draw_hit(CED_Hit *h){
    GLfloat d;
    CED_Point p_new = fisheye_transform(h->p.x, h->p.y, h->p.z, fisheye_alpha);
    float x = p_new.x;
    float y = p_new.y;
    float z = p_new.z;

    if(setting.phi_projection){
        //phi_projection is on
        y = y > 0 ? sqrt(x*x + y*y) : -1*sqrt(x*x + y*y);
        x = 0; 
    }
   
    if(setting.z_projection){
        z=0;
    }

    //printf("hit on layer: %i\n", h->layer);

    // time is passed to the hit data and is expected to be animated
    bool to_animate = h->time > 0.f;
    if ( to_animate && animate_layer == -1 ) animation_start_time = (int)SDL_GetTicks();

    if(!IS_VISIBLE(h->layer)){
        if (to_animate && animate_layer == int(h->layer) ) animate_layer = -1;
        return;
    }

    //    printf("Draw hit at : %f %f %f type = %d and ced_visible_layers = %d \n",h->p.x,h->p.y,h->p.z,h->type,ced_visible_layers);
    if (to_animate){
        if ( animate_layer == -1 ) animate_layer = h->layer;
        else if ( animate_layer != int(h->layer) ){
            setting.layer[animate_layer] = false;
            animate_layer = h->layer;
            animation_start_time = (int)SDL_GetTicks();
        }
        float elapsed_time = 0.001*( (int)SDL_GetTicks() - animation_start_time); // in seconds
        if ( elapsed_time < h->time ) return ;
    }


    ced_color(h->color);



    glDisable(GL_BLEND);

//    std::cout << " size=" << h->size << " screenshot_sections " << setting.screenshot_sections 
//	<< " type=" << h->type << std::endl;


    switch(h->type){

    	case CED_HIT_CROSS:
    	case CED_HIT_BOX:
    	case CED_HIT_VXD:
    	case CED_HIT_STAR:
    	    //glLineWidth(1.);
            glLineWidth(1.);

    	    glBegin(GL_LINES);

    	    if(h->type ==  CED_HIT_CROSS){
    	       	 //     printf("cross type == %d \n",(h->type & CED_HIT_CROSS));
//    	        d=h->size/2*setting.screenshot_sections;
    	        d=((GLfloat)h->size)/20.*setting.screenshot_sections;
    	        glVertex3f(x-d,y-d,z);
    	        glVertex3f(x+d,y+d,z);
		glVertex3f(x+d,y-d,z);
    	        glVertex3f(x-d,y+d,z);       
            }
    	    else if(h->type ==  CED_HIT_STAR){
    	       	 //     printf("cross type == %d \n",(h->type & CED_HIT_CROSS));
//    	        d=h->size/2*setting.screenshot_sections;
    	        d=((GLfloat)h->size)/20.*setting.screenshot_sections;

#if 0 // hauke's version
    	        glVertex3f(x-d,y-d,z+d);
    	        glVertex3f(x+d,y+d,z-d);
    
    	        glVertex3f(x+d,y-d,z+d);
    	        glVertex3f(x-d,y+d,z-d);
    
    	        glVertex3f(x+d,y+d,z+d);
    	        glVertex3f(x-d,y-d,z-d);
    
    	        glVertex3f(x-d,y+d,z+d);
    	        glVertex3f(x+d,y-d,z-d);
#else // vassillie's version
    	        glVertex3f(x-d,y,z);
    	        glVertex3f(x+d,y,z);
    
		glVertex3f(x,y-d,z);
    	        glVertex3f(x,y+d,z);

		glVertex3f(x,y,z-d);
    	        glVertex3f(x,y,z+d);
#endif
	    }
    	    else if(h->type ==  CED_HIT_VXD){
    	       	 //     printf("cross type == %d \n",(h->type & CED_HIT_CROSS));
//    	        d=h->size/2*setting.screenshot_sections;
//    	        d=((GLfloat)h->size)/20.*setting.screenshot_sections;
                d=0.005;
#if 1
    	        glVertex3f(x-d,y-d,z+d);
    	        glVertex3f(x+d,y+d,z-d);
    
    	        glVertex3f(x+d,y-d,z+d);
    	        glVertex3f(x-d,y+d,z-d);
    
    	        glVertex3f(x+d,y+d,z+d);
    	        glVertex3f(x-d,y-d,z-d);
    
    	        glVertex3f(x-d,y+d,z+d);
    	        glVertex3f(x+d,y-d,z-d);
#endif
//
//               double sizes[3]={0.1, 0.1, 0.1};
//               double center[3]={x, y, z};
//               ced_geobox( sizes, center, 0xff00ff );
//
#if 0
      std::cout << " calling ced_draw_geobox_r_solid..." << std::endl;
      CED_GeoBoxR abox;
      abox.sizes[0]=0.005 ; abox.sizes[1]=0.015; abox.sizes[2]=0.005;
      abox.center[0]=x; abox.center[1]=y; abox.center[2]=z; 
      abox.color=0xff00ff; 
      abox.rotate[0]=0.0 ; abox.rotate[1]=0.0;  abox.rotate[2]=0.0;
      ced_draw_geobox_r_solid(&abox);
#endif
    	    } else {
    	       	//      printf("star type == %d \n",(h->type & CED_HIT_STAR));
//    	        d=h->size/2.*setting.screenshot_sections;
    	        d=((GLfloat)h->size)/20.*setting.screenshot_sections;
    	        glVertex3f(x-d,y,z);
    	        glVertex3f(x+d,y,z);
    	        glVertex3f(x,y-d,z);
    	        glVertex3f(x,y+d,z);
    	        glVertex3f(x,y,z-d);
    	        glVertex3f(x,y,z+d); 
    	    }
            glEnd();
    	    break;
    	default:
    	    glPointSize((GLfloat)h->size*setting.screenshot_sections);
    	    glBegin(GL_POINTS);

    	    //glVertex3fv(&p_new.x);
            glVertex3f(x,y,z);
            glEnd();

    }


    //glEnd();
    glEnable(GL_BLEND);
    ced_add_objmap(&h->p,5,h->lcioID,h->layer,0);
}
