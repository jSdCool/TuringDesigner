#ifndef APP_HPP
#define APP_HPP

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#include <string>

// Load asset and initialized stuffs here
void init_app();

// The main loop (return false to end loop)
bool app_loop();

// Free up allocated memory
void deinit_app();

Font getModifiedFont();

void exportToRenderedImage(const std::string &fileName);

void loadMachine(std::string fileName);

#endif