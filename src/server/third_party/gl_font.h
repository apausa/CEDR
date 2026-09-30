#ifndef GL_FONT_H
#define GL_FONT_H

#define FONT_S 12
#define FONT_M 14
#define FONT_L 16

void font_init();
void font_clean();
void font_render(int font_id, float x, float y, const char* text);
int  font_get_height(int font_id);
int  font_get_width(int font_id, const char* text);

#endif /* GL_FONT_H */
