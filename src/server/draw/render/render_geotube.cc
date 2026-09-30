#include <fg_geometry.h>

#include "render.h"
#include "../geometry/fisheye.h"
#include "../ui/selection.h"
#include "../geometry/cylinder_geometry.h"

void ced_draw_geotube(CED_GeoTube *c){
    using namespace std;
    if(!IS_VISIBLE(c->type)){
        return;
    }

    //cout << "Detector layer: "  << c->type-NUMBER_DATA_LAYER << "trans: " << setting.detector_trans[c->type-NUMBER_DATA_LAYER] << endl;

    double transformed_shift = single_fisheye_transform(c->shift, fisheye_alpha);


    double cut_angle=setting.detector_cut_angle[c->type-NUMBER_DATA_LAYER];
    double trans_value=setting.detector_trans[c->type-NUMBER_DATA_LAYER];

    double cut_z=setting.detector_cut_z[c->type-NUMBER_DATA_LAYER];


    //SM-H: Fisheye code
    double d_o = single_fisheye_transform(c->r_o, fisheye_alpha);
    double d_i = single_fisheye_transform(c->r_i, fisheye_alpha);

    double z0 = transformed_shift;
    double z1 = single_fisheye_transform(c->z+c->shift, fisheye_alpha);
    double z = z1-z0;


   //z cutting
   //glColor4f(0.5, 0, 0, 0.5);
   //glBegin(GL_QUADS);
   //glVertex3f( -10000, 10000,-1*setting.z_cutting);
   //glVertex3f(  10000, 10000,-1*setting.z_cutting); 
   //glVertex3f(  10000,-10000,-1*setting.z_cutting); 
   //glVertex3f( -10000,-10000,-1*setting.z_cutting); 
   //glEnd();
//


    if(setting.detector_picking == true){
         double tmpz, tmpr;


         if(-1*cut_z < (transformed_shift)){
             //make 1line in the middle

             tmpz=z1; //make 1line in the middle
             tmpr=(d_i+d_o)/2.;
             for(double y=0;y<2*3.14-2*3.14*cut_angle/360.;y+=0.1){
                  //CED_Point tmp1;
                  //tmp1.x = d_o*sin(y);
                  //tmp1.y = d_o*cos(y);
                  //tmp1.z = z0;
                  //ced_add_objmap(&tmp1,5,0,0 );
                  ////ced_add_objmap(new CED_Point(d_o*sin(y), d_o*cos(y), z0),5,0,0 );

                  //z0 left end

                   ////z1 middle
                  //right z0-2*(z0-z1)
                  //ced_add_objmap(new CED_Point(tmpr*sin(y), tmpr*cos(y), tmpz),5,0,c->type,1);
                  CED_Point tmp2;
                  tmp2.x = tmpr*sin(y);
                  tmp2.y = tmpr*cos(y);
                  tmp2.z = tmpz;
                  ced_add_objmap(&tmp2,20,0,c->type,1 );

             } 
        }



        tmpz=z0;
        if(-1*cut_z > (transformed_shift)){
             tmpz=-1*cut_z;
        }

        for(; tmpz <= z0-2*(z0-z1); tmpz+=200){
             for(double tmprr=d_i; tmprr < d_o; tmprr+=600){
                 for(double y=0;y<2*3.14-2*3.14*cut_angle/360.;y+=0.20){
                      //z0 left end
                      //z1 middle
                      //right z0-2*(z0-z1)
                      CED_Point tmp2;
                      tmp2.x = tmprr*sin(y);
                      tmp2.y = tmprr*cos(y);
                      tmp2.z = tmpz;
                      ced_add_objmap(&tmp2,20,0,c->type,1 );

                 } 
             }

             for(double tmprr=d_i+300; tmprr < d_o; tmprr+=600){
                 for(double y=0.1;y<2*3.14-2*3.14*cut_angle/360.;y+=0.20){
                      //z0 left end
                      //z1 middle
                      //right z0-2*(z0-z1)
                      CED_Point tmp2;
                      tmp2.x = tmprr*sin(y);
                      tmp2.y = tmprr*cos(y);
                      tmp2.z = tmpz;
                      ced_add_objmap(&tmp2,20,0,c->type,1 );

                 } 
             }

        }
   }

   if(-1*cut_z > (transformed_shift+2*z)){
        //component is completly in outside range
      return; 
   }else if(-1*cut_z > (transformed_shift)){
      //some part of component is in cutting range
      double backup_transformed_shift=transformed_shift;
      transformed_shift=transformed_shift+(-1*cut_z-transformed_shift)/2.;
      z=z-(-1*cut_z-transformed_shift);
      transformed_shift=backup_transformed_shift+(-1*cut_z-backup_transformed_shift);
   }



    glPushMatrix();
    //hauke
    //if(graphic[1] == 1){
    if(setting.trans == 1){
        for(int k=0;k<2;k++){        

        //for(int k=1;k>=0;k--){        
            GLfloat face_color[4]={static_cast<GLfloat>(((c->color>>16)&0xff)/255.0),static_cast<GLfloat>(((c->color>>8)&0xff)/255.0),static_cast<GLfloat>(((c->color)&0xff)/255.0), static_cast<GLfloat>(trans_value)};
            //GLfloat face_color[4]={((c->color>>16)&0xff)/255.0/2.0+(1.0-setting.bgcolor[0])/2.0,((c->color>>8)&0xff)/255.0/2.0+(1.0-setting.bgcolor[1])/2.0,((c->color)&0xff)/255.0/2.0+(1.0-setting.bgcolor[2])/2.0, trans_value}; //shape in detector color mixed with anti background color
    
    
            //float detector_lines_wide=0.3;

//            float detector_lines_wide=CED_GEOTUBE_LINE_WIDTH;
            float detector_lines_wide=CED_GEOTUBE_LINE_WIDTH*setting.autoshot_scale;
            
    
            //GLfloat line_color[4]={0.5,0.5,0.5, 0.4}; //lines in gray
            //GLfloat line_color[4]={((c->color>>16)&0xff)/255.0,((c->color>>8)&0xff)/255.0,((c->color)&0xff)/255.0, 0.5}; //lines in detector color
            glGetDoublev(GL_COLOR_CLEAR_VALUE, setting.bgcolor);
            //GLfloat line_color[4]={((c->color>>16)&0xff)/255.0/2.0+(1.0-setting.bgcolor[0])/2.0,((c->color>>8)&0xff)/255.0/2.0+(1.0-setting.bgcolor[1])/2.0,((c->color)&0xff)/255.0/2.0+(1.0-setting.bgcolor[2])/2.0, 0.6}; //lines in detector color mixed with anti background color
    
            GLfloat line_color[4]={static_cast<GLfloat>(((c->color>>16)&0xff)/255.0/2.0+(1.0-setting.bgcolor[0])/2.0),static_cast<GLfloat>(((c->color>>8)&0xff)/255.0/2.0+(1.0-setting.bgcolor[1])/2.0),static_cast<GLfloat>(((c->color)&0xff)/255.0/2.0+(1.0-setting.bgcolor[2])/2.0), static_cast<GLfloat>((1-trans_value)+CED_GEOTUBE_LINE_MAX_TRANS)}; //lines in detector color mixed with anti background color
    
    
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //default
            //glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR); //glass
            glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA); //locks nice, but lines diapear, so switch it off after drawing
    
            glMatrixMode(GL_MODELVIEW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    

            

            if(cut_angle < 360){
                //std::cout << setting.z_cutting << " vs " << transformed_shift-z << std::endl;
               // if(-2*setting.z_cutting > transformed_shift+z+z){

               //if(-2*setting.z_cutting < transformed_shift){
               // if(fabs((-2*setting.z_cutting-transformed_shift)) < z){
               //     z-=fabs((-2*setting.z_cutting-transformed_shift));
               // }else{
               //     z=0;
               // }
               // transformed_shift-=fabs(-2*setting.z_cutting-transformed_shift)/2. ;
               // //z=-2*setting.z_cutting-transformed_shift+2*z;
               // }

                
                
                glTranslatef(0.0, 0.0, transformed_shift);
                if(c->rotate_o > 0.01 ) glRotatef(c->rotate_o, 0, 0, 1);
    
                if(c->rotate_o <= cut_angle){ //dont cut if rotate angle is to big
                    if(c->edges_o != c->edges_i || c->rotate_i != 0){
                        //glColor4f((c->color>>16)&0xff,(c->color>>8)&0xff,(c->color)&0xff, 0.8);
                        glColor4f(face_color[0], face_color[1], face_color[2], face_color[3]);
                        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    
    
                        //here
                        //draw the inner shape
                        glRotatef(c->rotate_i, 0, 0, 1);
     
                        glPolygonOffset( 1.f, 1.f );
                        if(trans_value < 1.0){
    
                            //std::cout << "call drawPartialCylinder outer:" << 0 << " inner" << 1 << std::endl;
                            if(k==0)
                                drawPartialCylinder(z*2, d_o-(d_o-d_i)/5, d_i, c->edges_i, cut_angle - c->rotate_i- c->rotate_o, c->rotate_i + c->rotate_o,0,1); //draw the inner cylinder 
                        }
    
                        glRotatef(-1*c->rotate_i, 0, 0, 1);
    
                        //draw the outer shape
                        glPolygonOffset( 1.f, 1.f ); 
                        glEnable( GL_POLYGON_OFFSET_FILL );
                        glPolygonOffset( 2.f, 2.f );
                        if(trans_value < 1.0){
                            //std::cout << "call drawPartialCylinder outer:" << 1 << " inner" << 0 << std::endl;
                            if(k==0)
                                drawPartialCylinder(z*2, d_o, d_i+(d_o-d_i)/5, c->edges_o, cut_angle - c->rotate_o, c->rotate_o,1,0, c->rotate_i); //draw the outer cylinder
                        }
    
    
    
    
    
    //                    //draw the inner shape
    //                    glRotatef(c->rotate_i, 0, 0, 1);
    //                    glPolygonOffset( 1.f, 1.f ); 
    //                    glEnable( GL_POLYGON_OFFSET_FILL );
    //                    glPolygonOffset( 2.f, 2.f );
    //                    if(trans_value < 1.0){
    //                        drawPartialCylinder(z*2, d_o-(d_o-d_i)/5, d_i, c->edges_i, cut_angle - c->rotate_i- c->rotate_o, c->rotate_i + c->rotate_o,0,1); //draw the inner cylinder 
    //                    }
    //                    //draw the outer shape
    //                    glRotatef(-1*c->rotate_i, 0, 0, 1);
    //                    glPolygonOffset( 1.f, 1.f );
    //                    if(trans_value < 1.0){
    //                        drawPartialCylinder(z*2, d_o, d_i+(d_o-d_i)/5, c->edges_o, cut_angle - c->rotate_o, c->rotate_o,1,0); //draw the outer cylinder
    //                    }
    //
    
                        //glLineWidth(0.5);
    
                        //glColor4f(0.5,0.5,0.5, 0.4);
                        if(c->type == unsigned(selected_layer) && setting.picking_highlight){
                            glLineWidth(detector_lines_wide*5);
                            glColor4f(1, 0,0,line_color[3]);
                        }else{ 
                            glLineWidth(detector_lines_wide);
                            glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        }
                        glRotatef(c->rotate_i, 0, 0, 1);
                        //draw the inner cylinder 
                        if(k==1)
                        drawPartialLineCylinder(z*2, d_o-(d_o-d_i)/5, d_i, c->edges_i, cut_angle - c->rotate_i- c->rotate_o, c->rotate_i + c->rotate_o,0,1); 
                        glRotatef(-1*c->rotate_i, 0, 0, 1);
                        //draw the outer cylinder

                        if(k==1)
                        drawPartialLineCylinder(z*2, d_o, d_i+(d_o-d_i)/5, c->edges_o, cut_angle - c->rotate_o, c->rotate_o,1,0);
                    }else{
    
                        //glColor4f(((c->color>>16)&0xff)*0.02,((c->color>>8)&0xff)*0.02,((c->color)&0xff)*0.02, 0.4);
                        //glLineWidth(5);
    
                        //glColor4f((c->color>>16)&0xff,(c->color>>8)&0xff,(c->color)&0xff, 0.8);
                       //glLineWidth(0.5);
                        glColor4f(face_color[0], face_color[1], face_color[2], face_color[3]);
    
                        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    
                        if(trans_value < 1.0){
    
                            //std::cout << "call drawPartialCylinder inner:" << 1 << " outer" << 1 << std::endl;

                            if(k==0)
                                drawPartialCylinder(z*2, d_o, d_i, c->edges_o, cut_angle - c->rotate_o, c->rotate_o);
                        }
    
                        //glLineWidth(1);
                        glLineWidth(detector_lines_wide);
    
    
                        //glColor4f(0.5,0.5,0.5, 0.4);
                        //glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        if(c->type == unsigned(selected_layer) && setting.picking_highlight){
                            glLineWidth(detector_lines_wide*5);
                            glColor4f(1, 0,0,line_color[3]);
                        }else{ 
                            glLineWidth(detector_lines_wide);
                            glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        }

    
                        //glColor4f(1,1,1, 0.0);
    

                        if(k==1)
                        drawPartialLineCylinder(z*2, d_o, d_i, c->edges_o, cut_angle - c->rotate_o, c->rotate_o);
                    }
                }else{
                    if(c->edges_o != c->edges_i || c->rotate_i != 0){
                        //glColor4f((c->color>>16)&0xff,(c->color>>8)&0xff,(c->color)&0xff, 0.8);
                        glColor4f(face_color[0], face_color[1], face_color[2], face_color[3]);
    
                        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    
    
    
    
    
                        //draw the inner shape
                        glRotatef(c->rotate_i, 0, 0, 1);
     
                        glPolygonOffset( 1.f, 1.f );
                        if(trans_value < 1.0){
    
                            //std::cout << "call drawPartialCylinder inner:" << 1 << " outer" << 0 << std::endl;

                            if(k==0)
                                drawPartialCylinder(z*2, d_o-(d_o-d_i)/5, d_i, c->edges_i, 0,0 ,0,1); //draw the inner cylinder 
                        }
    
                        glRotatef(-1*c->rotate_i, 0, 0, 1);
    
    
                        //draw the outer shape
                        glPolygonOffset( 1.f, 1.f ); 
                        glEnable( GL_POLYGON_OFFSET_FILL );
                        glPolygonOffset( 2.f, 2.f );
                        if(trans_value < 1.0){
    
                            //std::cout << "call drawPartialCylinder inner:" << 0 << " outer" << 1 << std::endl;
                            if(k==0)
                            drawPartialCylinder(z*2, d_o, d_i+(d_o-d_i)/5, c->edges_o, 0,0,1,0, c->rotate_i); //draw the outer cylinder
                        }
    
    
    
                        //glLineWidth(0.5);
                        //glLineWidth(1);
                        glLineWidth(detector_lines_wide);
    
    
                        //glColor4f(0.5,0.5,0.5, 0.4);
                        //glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        if(c->type == unsigned(selected_layer) && setting.picking_highlight){
                            glLineWidth(detector_lines_wide*5);
                            glColor4f(1, 0,0,line_color[3]);
                        }else{ 
                            glLineWidth(detector_lines_wide);
                            glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        }

    
    
                        glRotatef(c->rotate_i, 0, 0, 1);
                        //draw the inner cylinder 

                        if(k==1)
                        drawPartialLineCylinder(z*2, d_o-(d_o-d_i)/5, d_i, c->edges_i, 0,0,0,1); 
                        glRotatef(-1*c->rotate_i, 0, 0, 1);
                        //draw the outer cylinder

                        if(k==1)
                        drawPartialLineCylinder(z*2, d_o, d_i+(d_o-d_i)/5, c->edges_o, 0,0,1,0);
                    }else{
                        //glColor4f(((c->color>>16)&0xff)*0.02,((c->color>>8)&0xff)*0.02,((c->color)&0xff)*0.02, 0.4);
                        //glLineWidth(5);
    
                        //glColor4f((c->color>>16)&0xff,(c->color>>8)&0xff,(c->color)&0xff, 0.8);
    
                        glColor4f(face_color[0], face_color[1], face_color[2], face_color[3]);
    
                        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
    
                        if(trans_value < 1.0){
    
                            //std::cout << "call drawPartialCylinder inner:" << 1 << " outer" << 1 << std::endl;
                            if(k==0)
                            drawPartialCylinder(z*2, d_o, d_i, c->edges_o, 0,0);
                        }
    
                        //glLineWidth(0.5);
                        //glLineWidth(1);
                        glLineWidth(detector_lines_wide);
    
    
                        //glColor4f(0.5,0.5,0.5, 0.4);
                        glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        if(c->type == unsigned(selected_layer) && setting.picking_highlight){
                            glLineWidth(detector_lines_wide*5);
                            glColor4f(1, 0,0,line_color[3]);
                        }else{ 
                            glLineWidth(detector_lines_wide);
                            glColor4f(line_color[0], line_color[1], line_color[2], line_color[3]);
                        }

    
                        //glColor4f(1,1,1, 0.0);
    

                        if(k==1)
                            drawPartialLineCylinder(z*2, d_o, d_i, c->edges_o, 0,0);
                    }
                }

                glTranslatef(0.0, 0.0, -transformed_shift);
                glRotatef(-1*c->rotate_o, 0, 0, 1);

            }
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //default
    }
    //end trans drawing
    }else{ 
       //int maxEdges=20; //how many edges should be drawn in classic view maximal
       glLineWidth(1.);
       //glLineWidth(detector_lines_wide);

       ced_color(c->color);
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    
     
       glTranslatef(0.0, 0.0, transformed_shift);

       if(c->rotate_o > 0.01 ) glRotatef(c->rotate_o, 0, 0, 1);

        if(c->classic_outer){
            geoSolidCylinder(d_o, z*2, c->edges_o, 1); // @refactored: replace gluCylinder
        }
        if(d_i > 0 && c->classic_inner){
            if(c->rotate_o > 0.01 ) glRotatef(c->rotate_i, 0, 0, 1);
            geoSolidCylinder(d_i, z*2, c->edges_i, 1); // @refactored: replace gluCylinder
        }
    }
    glPopMatrix();
}
