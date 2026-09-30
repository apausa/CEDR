/*
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Fri Dec 3 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* geometry rendering */

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <fg_geometry.h>

#define GEOMETRY_EXIT_IF_NOT_INITIALISED(x)

static void fgError(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void fgWarning(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

static void fghDrawGeometrySolid11(GLfloat *vertices, GLfloat *normals, GLfloat *textcs, GLsizei numVertices,
                                   GLushort *vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);

    if (textcs)
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, textcs);
    }

    if (!vertIdxs)
        glDrawArrays(GL_TRIANGLES, 0, numVertices);
    else
        if (numParts>1)
            for (i=0; i<numParts; i++)
                glDrawElements(GL_TRIANGLE_STRIP, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs+i*numVertIdxsPerPart);
        else
            glDrawElements(GL_TRIANGLES, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (textcs)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

/*
 * Compute lookup table of cos and sin values forming a circle
 * (or half circle if halfCircle==TRUE)
 *
 * Notes:
 *    It is the responsibility of the caller to free these tables
 *    The size of the table is (n+1) to form a connected loop
 *    The last entry is exactly the same as the first
 *    The sign of n can be flipped to get the reverse loop
 */
static void fghCircleTable(GLfloat **sint, GLfloat **cost, const int n, const GLboolean halfCircle)
{
    int i;

    /* Table size, the sign of n flips the circle direction */
    const int size = abs(n);

    /* Determine the angle between samples */
    const GLfloat angle = (halfCircle?1:2)*(GLfloat)M_PI/(GLfloat)( ( n == 0 ) ? 1 : n );

    /* Allocate memory for n samples, plus duplicate of first entry at the end */
    *sint = (GLfloat *)malloc(sizeof(GLfloat) * (size+1));
    *cost = (GLfloat *)malloc(sizeof(GLfloat) * (size+1));

    /* Bail out if memory allocation fails, fgError never returns */
    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        fgError("Failed to allocate memory in fghCircleTable");
    }

    /* Compute cos and sin around the circle */
    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = (GLfloat)sin(angle*i);
        (*cost)[i] = (GLfloat)cos(angle*i);
    }


    if (halfCircle)
    {
        (*sint)[size] =  0.0f;  /* sin PI */
        (*cost)[size] = -1.0f;  /* cos PI */
    }
    else
    {
        /* Last sample is duplicate of the first (sin or cos of 2 PI) */
        (*sint)[size] = (*sint)[0];
        (*cost)[size] = (*cost)[0];
    }
}

static void fghGenerateSphere(GLfloat radius, GLint slices, GLint stacks, GLfloat **vertices, GLfloat **normals, int* nVert)
{
    int i,j;
    int idx = 0;    /* idx into vertex/normal buffer */
    GLfloat x,y,z;

    /* Pre-computed circle */
    GLfloat *sint1,*cost1;
    GLfloat *sint2,*cost2;

    /* number of unique vertices */
    if (slices==0 || stacks<2)
    {
        /* nothing to generate */
        *nVert = 0;
        return;
    }
    *nVert = slices*(stacks-1)+2;
    if ((*nVert) > 65535)
        /*
         * limit of glushort, that's 256*256 subdivisions, should be enough in practice. See note above
         */
        fgWarning("fghGenerateSphere: too many slices or stacks requested, indices will wrap");

    /* precompute values on unit circle */
    fghCircleTable(&sint1,&cost1,-slices,GL_FALSE);
    fghCircleTable(&sint2,&cost2, stacks,GL_TRUE);

    /* Allocate vertex and normal buffers, bail out if memory allocation fails */
    *vertices = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    *normals  = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
        fgError("Failed to allocate memory in fghGenerateSphere");
    }

    /* top */
    (*vertices)[0] = 0.f;
    (*vertices)[1] = 0.f;
    (*vertices)[2] = radius;
    (*normals )[0] = 0.f;
    (*normals )[1] = 0.f;
    (*normals )[2] = 1.f;
    idx = 3;

    /* each stack */
    for( i=1; i<stacks; i++ )
    {
        for(j=0; j<slices; j++, idx+=3)
        {
            x = cost1[j]*sint2[i];
            y = sint1[j]*sint2[i];
            z = cost2[i];

            (*vertices)[idx  ] = x*radius;
            (*vertices)[idx+1] = y*radius;
            (*vertices)[idx+2] = z*radius;
            (*normals )[idx  ] = x;
            (*normals )[idx+1] = y;
            (*normals )[idx+2] = z;
        }
    }

    /* bottom */
    (*vertices)[idx  ] =  0.f;
    (*vertices)[idx+1] =  0.f;
    (*vertices)[idx+2] = -radius;
    (*normals )[idx  ] =  0.f;
    (*normals )[idx+1] =  0.f;
    (*normals )[idx+2] = -1.f;

    /* Done creating vertices, release sin and cos tables */
    free(sint1);
    free(cost1);
    free(sint2);
    free(cost2);
}

void fghGenerateCone(GLfloat base, GLfloat height, GLint slices, GLint stacks,
        GLfloat **vertices, GLfloat **normals, int* nVert)
{
    int i,j;
    int idx = 0;    /* idx into vertex/normal buffer */

    /* Pre-computed circle */
    GLfloat *sint,*cost;

    /* Step in z and radius as stacks are drawn. */
    GLfloat z = 0;
    GLfloat r = (GLfloat)base;

    const GLfloat zStep = (GLfloat)height / ( ( stacks > 0 ) ? stacks : 1 );
    const GLfloat rStep = (GLfloat)base / ( ( stacks > 0 ) ? stacks : 1 );

    /* Scaling factors for vertex normals */
    const GLfloat cosn = (GLfloat) (height / sqrt( height * height + base * base ));
    const GLfloat sinn = (GLfloat) (base   / sqrt( height * height + base * base ));



    /* number of unique vertices */
    if (slices==0 || stacks<1)
    {
        /* nothing to generate */
        *nVert = 0;
        return;
    }
    *nVert = slices*(stacks+2)+1;   /* need an extra stack for closing off bottom with correct normals */

    if ((*nVert) > 65535)
        /*
         * limit of glushort, that's 256*256 subdivisions, should be enough in practice. See note above
         */
        fgWarning("fghGenerateCone: too many slices or stacks requested, indices will wrap");

    /* Pre-computed circle */
    fghCircleTable(&sint,&cost,-slices,GL_FALSE);

    /* Allocate vertex and normal buffers, bail out if memory allocation fails */
    *vertices = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    *normals  = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
        fgError("Failed to allocate memory in fghGenerateCone");
    }

    /* bottom */
    (*vertices)[0] =  0.f;
    (*vertices)[1] =  0.f;
    (*vertices)[2] =  z;
    (*normals )[0] =  0.f;
    (*normals )[1] =  0.f;
    (*normals )[2] = -1.f;
    idx = 3;
    /* other on bottom (get normals right) */
    for (j=0; j<slices; j++, idx+=3)
    {
        (*vertices)[idx  ] = cost[j]*r;
        (*vertices)[idx+1] = sint[j]*r;
        (*vertices)[idx+2] = z;
        (*normals )[idx  ] =  0.f;
        (*normals )[idx+1] =  0.f;
        (*normals )[idx+2] = -1.f;
    }

    /* each stack */
    for (i=0; i<stacks+1; i++ )
    {
        for (j=0; j<slices; j++, idx+=3)
        {
            (*vertices)[idx  ] = cost[j]*r;
            (*vertices)[idx+1] = sint[j]*r;
            (*vertices)[idx+2] = z;
            (*normals )[idx  ] = cost[j]*cosn;
            (*normals )[idx+1] = sint[j]*cosn;
            (*normals )[idx+2] = sinn;
        }

        z += zStep;
        r -= rStep;
    }

    /* Release sin and cos tables */
    free(sint);
    free(cost);
}

void fghGenerateCylinder(GLfloat radius, GLfloat height, GLint slices, GLint stacks,
        GLfloat **vertices, GLfloat **normals, int* nVert)
{
    int i,j;
    int idx = 0;    /* idx into vertex/normal buffer */

    /* Step in z as stacks are drawn. */
    GLfloat radf = (GLfloat)radius;
    GLfloat z;
    const GLfloat zStep = (GLfloat)height / ( ( stacks > 0 ) ? stacks : 1 );
    
    /* Pre-computed circle */
    GLfloat *sint,*cost;

    /* number of unique vertices */
    if (slices==0 || stacks<1)
    {
        /* nothing to generate */
        *nVert = 0;
        return;
    }
    *nVert = slices*(stacks+3)+2;   /* need two extra stacks for closing off top and bottom with correct normals */

    if ((*nVert) > 65535)
        /*
         * limit of glushort, that's 256*256 subdivisions, should be enough in practice. See note above
         */
        fgWarning("fghGenerateCylinder: too many slices or stacks requested, indices will wrap");

    /* Pre-computed circle */
    fghCircleTable(&sint,&cost,-slices,GL_FALSE);

    /* Allocate vertex and normal buffers, bail out if memory allocation fails */
    *vertices = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    *normals  = (GLfloat *)malloc((*nVert)*3*sizeof(GLfloat));
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
        fgError("Failed to allocate memory in fghGenerateCylinder");
    }

    z=0;
    /* top on Z-axis */
    (*vertices)[0] =  0.f;
    (*vertices)[1] =  0.f;
    (*vertices)[2] =  0.f;
    (*normals )[0] =  0.f;
    (*normals )[1] =  0.f;
    (*normals )[2] = -1.f;
    idx = 3;
    /* other on top (get normals right) */
    for (j=0; j<slices; j++, idx+=3)
    {
        (*vertices)[idx  ] = cost[j]*radf;
        (*vertices)[idx+1] = sint[j]*radf;
        (*vertices)[idx+2] = z;
        (*normals )[idx  ] = 0.f;
        (*normals )[idx+1] = 0.f;
        (*normals )[idx+2] = -1.f;
    }

    /* each stack */
    for (i=0; i<stacks+1; i++ )
    {
        for (j=0; j<slices; j++, idx+=3)
        {
            (*vertices)[idx  ] = cost[j]*radf;
            (*vertices)[idx+1] = sint[j]*radf;
            (*vertices)[idx+2] = z;
            (*normals )[idx  ] = cost[j];
            (*normals )[idx+1] = sint[j];
            (*normals )[idx+2] = 0.f;
        }

        z += zStep;
    }

    /* other on bottom (get normals right) */
    z -= zStep;
    for (j=0; j<slices; j++, idx+=3)
    {
        (*vertices)[idx  ] = cost[j]*radf;
        (*vertices)[idx+1] = sint[j]*radf;
        (*vertices)[idx+2] = z;
        (*normals )[idx  ] = 0.f;
        (*normals )[idx+1] = 0.f;
        (*normals )[idx+2] = 1.f;
    }

    /* bottom */
    (*vertices)[idx  ] =  0.f;
    (*vertices)[idx+1] =  0.f;
    (*vertices)[idx+2] =  height;
    (*normals )[idx  ] =  0.f;
    (*normals )[idx+1] =  0.f;
    (*normals )[idx+2] =  1.f;

    /* Release sin and cos tables */
    free(sint);
    free(cost);
}

static void fghSphere( GLfloat radius, GLint slices, GLint stacks )
{
    int i,j,idx, nVert;
    GLfloat *vertices, *normals;

    /* Generate vertices and normals */
    fghGenerateSphere(radius,slices,stacks,&vertices,&normals,&nVert);

    if (nVert==0)
        /* nothing to draw */
        return;

    /* First, generate vertex index arrays for drawing with glDrawElements
     * All stacks, including top and bottom are covered with a triangle
     * strip.
     */
    GLushort  *stripIdx;
    /* Create index vector */
    GLushort offset;

    /* Allocate buffers for indices, bail out if memory allocation fails */
    stripIdx = (GLushort *)malloc((slices+1)*2*(stacks)*sizeof(GLushort));
    if (!(stripIdx))
    {
        free(stripIdx);
        fgError("Failed to allocate memory in fghSphere");
    }

    /* top stack */
    for (j=0, idx=0;  j<slices;  j++, idx+=2)
    {
        stripIdx[idx  ] = j+1;              /* 0 is top vertex, 1 is first for first stack */
        stripIdx[idx+1] = 0;
    }
    stripIdx[idx  ] = 1;                    /* repeat first slice's idx for closing off shape */
    stripIdx[idx+1] = 0;
    idx+=2;

    /* middle stacks: */
    /* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
    for (i=0; i<stacks-2; i++, idx+=2)
    {
        offset = 1+i*slices;                    /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
        for (j=0; j<slices; j++, idx+=2)
        {
            stripIdx[idx  ] = offset+j+slices;
            stripIdx[idx+1] = offset+j;
        }
        stripIdx[idx  ] = offset+slices;        /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = offset;
    }

    /* bottom stack */
    offset = 1+(stacks-2)*slices;               /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
    for (j=0; j<slices; j++, idx+=2)
    {
        stripIdx[idx  ] = nVert-1;              /* zero based index, last element in array (bottom vertex)... */
        stripIdx[idx+1] = offset+j;
    }
    stripIdx[idx  ] = nVert-1;                  /* repeat first slice's idx for closing off shape */
    stripIdx[idx+1] = offset;


    /* draw */
    fghDrawGeometrySolid11(vertices,normals,NULL,nVert,stripIdx,stacks,(slices+1)*2);

    /* cleanup allocated memory */
    free(stripIdx);
    free(vertices);
    free(normals);
}

static void fghCone( GLfloat base, GLfloat height, GLint slices, GLint stacks )
{
    int i,j,idx, nVert;
    GLfloat *vertices, *normals;

    /* Generate vertices and normals */
    /* Note, (stacks+1)*slices vertices for side of object, slices+1 for top and bottom closures */
    fghGenerateCone(base,height,slices,stacks,&vertices,&normals,&nVert);

    if (nVert==0)
        /* nothing to draw */
        return;

    /* First, generate vertex index arrays for drawing with glDrawElements
     * All stacks, including top and bottom are covered with a triangle
     * strip.
     */
    GLushort  *stripIdx;
    /* Create index vector */
    GLushort offset;

    /* Allocate buffers for indices, bail out if memory allocation fails */
    stripIdx = (GLushort *)malloc((slices+1)*2*(stacks+1)*sizeof(GLushort));    /*stacks +1 because of closing off bottom */
    if (!(stripIdx))
    {
        free(stripIdx);
        fgError("Failed to allocate memory in fghCone");
    }

    /* top stack */
    for (j=0, idx=0;  j<slices;  j++, idx+=2)
    {
        stripIdx[idx  ] = 0;
        stripIdx[idx+1] = j+1;              /* 0 is top vertex, 1 is first for first stack */
    }
    stripIdx[idx  ] = 0;                    /* repeat first slice's idx for closing off shape */
    stripIdx[idx+1] = 1;
    idx+=2;

    /* middle stacks: */
    /* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
    for (i=0; i<stacks; i++, idx+=2)
    {
        offset = 1+(i+1)*slices;                /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
        for (j=0; j<slices; j++, idx+=2)
        {
            stripIdx[idx  ] = offset+j;
            stripIdx[idx+1] = offset+j+slices;
        }
        stripIdx[idx  ] = offset;               /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = offset+slices;
    }

    /* draw */
    fghDrawGeometrySolid11(vertices,normals,NULL,nVert,stripIdx,stacks+1,(slices+1)*2);

    /* cleanup allocated memory */
    free(stripIdx);
    free(vertices);
    free(normals);
}

static void fghCylinder( GLfloat radius, GLfloat height, GLint slices, GLint stacks )
{
    int i,j,idx, nVert;
    GLfloat *vertices, *normals;

    /* Generate vertices and normals */
    /* Note, (stacks+1)*slices vertices for side of object, 2*slices+2 for top and bottom closures */
    fghGenerateCylinder(radius,height,slices,stacks,&vertices,&normals,&nVert);

    if (nVert==0)
        /* nothing to draw */
        return;

    /* First, generate vertex index arrays for drawing with glDrawElements
    * All stacks, including top and bottom are covered with a triangle
    * strip.
    */
    GLushort  *stripIdx;
    /* Create index vector */
    GLushort offset;

    /* Allocate buffers for indices, bail out if memory allocation fails */
    stripIdx = (GLushort *)malloc((slices+1)*2*(stacks+2)*sizeof(GLushort));    /*stacks +2 because of closing off bottom and top */
    if (!(stripIdx))
    {
        free(stripIdx);
        fgError("Failed to allocate memory in fghCylinder");
    }

    /* top stack */
    for (j=0, idx=0;  j<slices;  j++, idx+=2)
    {
        stripIdx[idx  ] = 0;
        stripIdx[idx+1] = j+1;              /* 0 is top vertex, 1 is first for first stack */
    }
    stripIdx[idx  ] = 0;                    /* repeat first slice's idx for closing off shape */
    stripIdx[idx+1] = 1;
    idx+=2;

        /* middle stacks: */
        /* Strip indices are relative to first index belonging to strip, NOT relative to first vertex/normal pair in array */
    for (i=0; i<stacks; i++, idx+=2)
    {
        offset = 1+(i+1)*slices;                /* triangle_strip indices start at 1 (0 is top vertex), and we advance one stack down as we go along */
        for (j=0; j<slices; j++, idx+=2)
        {
            stripIdx[idx  ] = offset+j;
            stripIdx[idx+1] = offset+j+slices;
        }
        stripIdx[idx  ] = offset;               /* repeat first slice's idx for closing off shape */
        stripIdx[idx+1] = offset+slices;
    }

    /* top stack */
    offset = 1+(stacks+2)*slices;
    for (j=0; j<slices; j++, idx+=2)
    {
        stripIdx[idx  ] = offset+j;
        stripIdx[idx+1] = nVert-1;              /* zero based index, last element in array (bottom vertex)... */
    }
    stripIdx[idx  ] = offset;
    stripIdx[idx+1] = nVert-1;                  /* repeat first slice's idx for closing off shape */

    /* draw */
    fghDrawGeometrySolid11(vertices,normals,NULL,nVert,stripIdx,stacks+2,(slices+1)*2);

    /* cleanup allocated memory */
    free(stripIdx);
    free(vertices);
    free(normals);
}

void geoSolidSphere(double radius, int slices, int stacks)
{
    GEOMETRY_EXIT_IF_NOT_INITIALISED ( "geoSolidSphere" );
    fghSphere((GLfloat)radius, slices, stacks);
}

void geoSolidCone(double base, double height, int slices, int stacks)
{
    GEOMETRY_EXIT_IF_NOT_INITIALISED ( "geoSolidCone" );
    fghCone((GLfloat)base, (GLfloat)height, slices, stacks);
}

void geoSolidCylinder(double radius, double height, int slices, int stacks)
{
    GEOMETRY_EXIT_IF_NOT_INITIALISED ( "geoSolidCylinder" );
    fghCylinder((GLfloat)radius, (GLfloat)height, slices, stacks);
}
