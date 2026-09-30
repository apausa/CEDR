#pragma once

#include <SDL3/SDL.h>

void input_data(void *data);
SDL_HitTestResult SDLCALL ced_window_hit_test(SDL_Window *win, const SDL_Point *pt, void *data);
void mainLoop(SDL_GLContext gl_context);
