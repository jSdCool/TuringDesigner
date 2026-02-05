#ifndef HELPER_HPP
#define HELPER_HPP


#include <string>

#include "../libs/raylib/src/raylib.h"

int getWindowWidth();
int getWindowHeight();
int getBrowserWindowWidth();
int getBrowserWindowHeight();
void DrawTextCentered(std::string &text, int posX, int posY, int fontSize, Color color);
void DrawLineBezier(Vector2 start, Vector2 mid, Vector2 end, float thickness, Color color);
void DrawLineBezier(Vector2 start, Vector2 mid1, Vector2 mid2, Vector2 end, float thickness, Color color);
Vector2 quadritcLerp(Vector2 start, Vector2 mid, Vector2 end, float t);
Vector2 cubicLerp(Vector2 start, Vector2 mid1, Vector2 mid2, Vector2 end, float t);
void DrawText(Font font,const char *text, int posX, int posY, int fontSize, Color color);

#endif
