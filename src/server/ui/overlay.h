#pragma once

#include <string>

static constexpr int CED_TITLE_BAR_HEIGHT = 24;

void printFPS(void);
void printEventTime(void);
void printShortcuts(void);
void draw_ced_title_bar(void);

std::string truncateTo(std::string str, size_t max_len);
