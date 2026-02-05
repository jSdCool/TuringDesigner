#pragma once
#include "raylib.h"


class State {
    float x;
    float y;
    int id;
public:
    State(int x, int y, int id);

    void draw(float scale,Vector2 offset,bool highlight) const;
    [[nodiscard]] bool mouseOver(float x, float y) const;
    void updatePosition(int x, int y);
    Vector2 getPosition() const;
};


