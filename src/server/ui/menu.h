#pragma once

#include <string>

void buildMainMenu(void);
void buildPopUpMenu(int x, int y);
void buildLayerMenus(void);

std::string formatSubmenuEntry(int iLayer, const char key,
                                const char *description, size_t max_len);
