#include "render.h"
#include "../geometry/fisheye.h"
#include "../ui/selection.h"

/*
 * Line element
 */

void ced_draw_line(CED_Line *h){
    if(!IS_VISIBLE(h->type)){ //<< CED_LAYER_SHIFT)){
        return;
    }

//    std::cout << " CED_Line p0=" << h->p0.x << " ," << h->p0.y << " ," << h->p0.z << std::endl;
//    std::cout << " CED_Line p1=" << h->p1.x << " ," << h->p1.y << " ," << h->p1.z << std::endl;
//    std::cout << " CED screenshot_sections=" << setting.screenshot_sections << std::endl;
//    std::cout << " winx, y=" << setting.win_w << " ," << setting.win_h << 
//	 " zoom=" << setting.zoom << std::endl;
//    double length=pow(pow(h->p0.x-h->p1.x,2)+pow(h->p0.y-h->p1.y,2)+pow(h->p0.z-h->p1.z,2),0.5);
//    if( length < 1.0 ) { return ; }

    CED_Point fisheye_point0;
    CED_Point fisheye_point1;
    fisheye_point0 = fisheye_transform(h->p0.x, h->p0.y, h->p0.z, fisheye_alpha);
    fisheye_point1 = fisheye_transform(h->p1.x, h->p1.y, h->p1.z, fisheye_alpha);    	

   	//glEnable(GL_BLEND);


  	ced_color(h->color);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glMatrixMode(GL_MODELVIEW);

    //TODO
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glDisable(GL_BLEND);

    //glColor4f((h->color>>16)&0xff,(h->color>>8)&0xff,(h->color)&0xff, 1);
    //glColor3f((h->color>>16)&0xff,(h->color>>8)&0xff,(h->color)&0xff);


    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //end hauke
  	glLineWidth(h->width);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  	glBegin(GL_LINES);


    
#if 1
    //hauke
    if(setting.phi_projection){
      //phi_projection is on
        fisheye_point0.y = fisheye_point0.y > 0 ? sqrt(fisheye_point0.x*fisheye_point0.x + fisheye_point0.y*fisheye_point0.y) : -1*sqrt(fisheye_point0.x*fisheye_point0.x + fisheye_point0.y*fisheye_point0.y);
        fisheye_point0.x = 0;

        fisheye_point1.y = fisheye_point1.y > 0 ? sqrt(fisheye_point1.x*fisheye_point1.x + fisheye_point1.y*fisheye_point1.y) : -1*sqrt(fisheye_point1.x*fisheye_point1.x + fisheye_point1.y*fisheye_point1.y);
        fisheye_point1.x = 0;
   }
   if(setting.z_projection){
    fisheye_point0.z=0;
    fisheye_point1.z=0;
   }
   //end hauke

    glVertex3fv(&fisheye_point0.x); 
    glVertex3fv(&fisheye_point1.x); 

#endif
//      glVertex3fv(&(h->p0.x));
//      glVertex3fv(&(h->p1.x));

  	//glDisable(GL_BLEND);
  	glEnd();

    //glEnable(GL_BLEND);
    ced_add_objmap(&h->p0,5,h->lcioID,h->type,0 );
    ced_add_objmap(&h->p1,5,h->lcioID,h->type,0);

#if 0

//    double length=pow(pow(h->p0.x-h->p1.x,2)+pow(h->p0.y-h->p1.y,2)+pow(h->p0.z-h->p1.z,2),0.5);


    CED_Point p[100];
    double picking_point_space=5;
    int steps;

    if(int(length/picking_point_space) < 100){
      steps=int(length/picking_point_space);
    }else{
        steps=100;
    }

//    std::cout << " ced_draw_line ... length=" << length << " steps =" << steps << std::endl; 
#endif

#if 0 
// off for debug
    for(int i=0;i<steps;i++){
        p[i].x=(h->p1.x - h->p0.x)/steps*i+h->p0.x;
        p[i].y=(h->p1.y - h->p0.y)/steps*i+h->p0.y;
        p[i].z=(h->p1.z - h->p0.z)/steps*i+h->p0.z;
        ced_add_objmap(&p[i],5,h->lcioID, h->type,0);

        //display picking points for testing
        //glPointSize(5);
        //glBegin(GL_POINTS);
        //glVertex3f(p.x,p.y,p.z);
        //glEnd();
    }
#endif
    
}
