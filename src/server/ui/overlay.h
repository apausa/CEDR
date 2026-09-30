#pragma once

#include <string>

#include <ced_cli.h>

void printFPS(void);
void printEventTime(void);
void printShortcuts(void);
void draw_ced_title_bar(void);
void ced_draw_legend(CED_Legend *legend);

std::string truncateTo(std::string str, size_t max_len);
