#include "State.h"

#include <iostream>
#include <ostream>

#include "raylib.h"
#include "helper.hpp"

State::State(int x, int y, int id) {
    this->x=static_cast<float>(x);
    this->y=static_cast<float>(y);
    this->id=id;
}

State::State(nlohmann::json fileData, int id) {
    x = fileData["x"];
    y = fileData["y"];
    this->id = id;
}

void State::draw(float scale, Vector2 offset,bool highlight) const {
    Color outlineColor = highlight ? YELLOW : BLACK;
    Vector2 center = {(x+offset.x)*scale,(y+offset.y)*scale};
    DrawCircleV(center,30*scale,WHITE);
    DrawRing(center,29*scale,31*scale,0,360,100,outlineColor);

    int textSize = static_cast<int>(23 * scale);
    std::string idString = std::to_string(id);
    DrawTextCentered(idString,static_cast<int>(center.x),static_cast<int>(center.y)- textSize/2,textSize,BLACK);
}

bool State::mouseOver(float x, float y) const {
    if (sqrt(pow(x - this->x,2) + pow(y - this->y,2)) < 30) {
        return true;
    }
    return false;
}

void State::updatePosition(int x, int y) {
    this->x=static_cast<float>(x);
    this->y=static_cast<float>(y);
}

Vector2 State::getPosition() const {
    return {this->x,this->y};
}

nlohmann::json State::asJson() {
    nlohmann::json data{};
    data["x"] = x;
    data["y"] = y;
    return data;
}
