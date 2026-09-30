/*
  glfont:  An example of using the SDL_ttf library with OpenGL.
  Copyright (C) 2001-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#include <gl_font.h>

static TTF_Font *_font_s = NULL;
static TTF_Font *_font_m = NULL;
static TTF_Font *_font_l = NULL;

static void _SDL_GL_Enter2DMode(int width, int height)
{
    /* Note, there may be other things you need to change,
       depending on how you have your OpenGL state set up.
    */
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    /* This allows alpha blending of 2D textures with the scene */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, (GLdouble)width, (GLdouble)height, 0.0, 0.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

static void _SDL_GL_Leave2DMode(void)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

/* Quick utility function for texture creation */
static int _power_of_two(int input)
{
    int value = 1;

    while (value < input) {
        value <<= 1;
    }
    return value;
}

static GLuint SDL_GL_LoadTexture(SDL_Surface *surface, GLfloat *texcoord)
{
    GLuint texture;
    int w, h;
    SDL_Surface *image;
    SDL_Rect area;
    Uint8  saved_alpha;
    SDL_BlendMode saved_mode;

    /* Use the surface width and height expanded to powers of 2 */
    w = _power_of_two(surface->w);
    h = _power_of_two(surface->h);
    texcoord[0] = 0.0f;         /* Min X */
    texcoord[1] = 0.0f;         /* Min Y */
    texcoord[2] = (GLfloat)surface->w / w;  /* Max X */
    texcoord[3] = (GLfloat)surface->h / h;  /* Max Y */

    image = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    if (image == NULL) {
        return 0;
    }

    /* Save the alpha blending attributes */
    SDL_GetSurfaceAlphaMod(surface, &saved_alpha);
    SDL_SetSurfaceAlphaMod(surface, 0xFF);
    SDL_GetSurfaceBlendMode(surface, &saved_mode);
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);

    /* Copy the surface into the GL texture image */
    area.x = 0;
    area.y = 0;
    area.w = surface->w;
    area.h = surface->h;
    SDL_BlitSurface(surface, &area, image, &area);

    /* Restore the alpha blending attributes */
    SDL_SetSurfaceAlphaMod(surface, saved_alpha);
    SDL_SetSurfaceBlendMode(surface, saved_mode);

    /* Create an OpenGL texture for the image */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
             0,
             GL_RGBA,
             w, h,
             0,
             GL_RGBA,
             GL_UNSIGNED_BYTE,
             image->pixels);
    SDL_DestroySurface(image); /* No longer needed */

    return texture;
}

static const char *_font_resolve(void)
{
    static char path[1024];
    FcPattern *pat = FcNameParse((const FcChar8 *)"monospace"); // Write query
    
    FcConfigSubstitute(NULL, pat, FcMatchPattern); // Increase the scope of available fonts
    FcDefaultSubstitute(pat);

    FcResult result;
    FcPattern *match = FcFontMatch(NULL, pat, &result); // Query font

    if (match) {
        FcChar8 *file;

        if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch)
            snprintf(path, sizeof(path), "%s", (char *)file);

        FcPatternDestroy(match);
    }

    FcPatternDestroy(pat); // Function called for every pattern created

    return path[0] ? path : NULL;
}

// Map a font ID to its loaded TTF_Font
static TTF_Font *_font_get(int font_id)
{
    switch (font_id) {
        case FONT_S: return _font_s;
        case FONT_M: return _font_m;
        default: return _font_l;
    }
}

void font_init()
{
    if (!TTF_Init()) {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
        return;
    }

    const char *path = _font_resolve();

    if (!path) {
        fprintf(stderr, "fontconfig: could not find monospace font\n");
        return;
    }

    _font_s = TTF_OpenFont(path, FONT_S);
    _font_m = TTF_OpenFont(path, FONT_M);
    _font_l = TTF_OpenFont(path, FONT_L);
}

void font_render(int font_id, float x, float y, const char *text)
{
    GLfloat col[4];
    TTF_Font *font = _font_get(font_id);

    glGetFloatv(GL_CURRENT_COLOR, col); // read the current GL draw color

    SDL_Color color = {(Uint8)(col[0]*255), (Uint8)(col[1]*255), (Uint8)(col[2]*255), 255};
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color); // rasterize text to a CPU surface

    if (!surf) return;

    int w = surf->w, h = surf->h; // pixel dimensions of the rendered text
    GLfloat texcoord[4];
    GLuint texture = SDL_GL_LoadTexture(surf, texcoord); // upload surface to a GL texture
    SDL_DestroySurface(surf); // free surface from CPU
    
    if (!texture) return;

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp); // get current viewport to set up 2D projection

    /* Show the text on the screen */
    _SDL_GL_Enter2DMode(vp[2], vp[3]);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(texcoord[0], texcoord[1]); glVertex2f(x, y);
    glTexCoord2f(texcoord[2], texcoord[1]); glVertex2f(x+w, y);
    glTexCoord2f(texcoord[0], texcoord[3]); glVertex2f(x, y+h);
    glTexCoord2f(texcoord[2], texcoord[3]); glVertex2f(x+w, y+h);
    glEnd();
    glDeleteTextures(1, &texture);
    _SDL_GL_Leave2DMode();
}

int font_get_height(int font_id)
{
    TTF_Font *font = _font_get(font_id);

    int w = 0, h = 0;
    TTF_GetStringSize(font, "A", 0, &w, &h);

    return h;
}

int font_get_width(int font_id, const char *text)
{
    TTF_Font *font = _font_get(font_id);

    int w = 0, h = 0;
    TTF_GetStringSize(font, text, 0, &w, &h);

    return w;
}

void font_clean()
{
    TTF_CloseFont(_font_s);
    _font_s = NULL;

    TTF_CloseFont(_font_m);
    _font_m = NULL;

    TTF_CloseFont(_font_l);
    _font_l = NULL;

    TTF_Quit();
    FcFini();
}