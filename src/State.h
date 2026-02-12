#pragma once
#include "json.hpp"
#include "raylib.h"


class State {
    float x;
    float y;
    int id;
public:
    State(int x, int y, int id);

    State(nlohmann::json fileData, int id);

    void draw(float scale,Vector2 offset,Color color) const;
    [[nodiscard]] bool mouseOver(float x, float y) const;
    void updatePosition(int x, int y);
    [[nodiscard]] Vector2 getPosition() const;
    void decreaseId() {
        id --;
    }

    nlohmann::json asJson();
};


