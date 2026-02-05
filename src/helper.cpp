#include <string>

#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"

#include <cmath>
#include <vector>


int getWindowWidth() {
    if (IsWindowFullscreen()) {
        return GetMonitorWidth(GetCurrentMonitor());
    } else {
        return GetScreenWidth();
    }
}

int getWindowHeight() {
    if (IsWindowFullscreen()) {
        return GetMonitorHeight(GetCurrentMonitor());
    }
    else {
        return GetScreenHeight();
    }
}

int getBrowserWindowWidth() {
    return getWindowWidth();
}

int getBrowserWindowHeight() {
    return getWindowHeight();
}

// Draw text centered horizontally. Included because I use it all the time.
void DrawTextCentered(std::string &text, int posX, int posY, int fontSize, Color color) {
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), posX-(textWidth/2), posY, fontSize, color);
}

Vector2 quadritcLerp(const Vector2 start, const Vector2 mid, const Vector2 end, const float t) {
    const Vector2 lerp1 = {std::lerp(start.x,mid.x,t),std::lerp(start.y,mid.y,t)};
    const Vector2 lerp2 = {std::lerp(mid.x,end.x,t),std::lerp(mid.y,end.y,t)};
    return {std::lerp(lerp1.x,lerp2.x,t),std::lerp(lerp1.y,lerp2.y,t)};
}

void DrawLineBezier(const Vector2 start, const Vector2 mid, const Vector2 end, const float thickness, const Color color) {
    Vector2 previous = start;
    Vector2 current = { 0 };

    constexpr int SEGMENTS = 24;

    std::vector <Vector2> points;
    points.reserve(2*SEGMENTS + 2);
    for (int i = 0; i < 2*SEGMENTS + 2; i++) {
        points.push_back({0,0});
    }

    for (int i = 1; i <= 24; i++)
    {
        // Quadric curv lerping
        const float t = static_cast<float>(i) / SEGMENTS;
        current  = quadritcLerp(start, mid, end, t);

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thickness/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points.data(), 2*SEGMENTS + 2, color);
}

Vector2 cubicLerp(const Vector2 start, const Vector2 mid1, const Vector2 mid2, const Vector2 end, const float t) {
    const Vector2 lerp1 = {std::lerp(start.x,mid1.x,t),std::lerp(start.y,mid1.y,t)};
    const Vector2 lerp2 = {std::lerp(mid1.x,mid2.x,t),std::lerp(mid1.y,mid2.y,t)};
    const Vector2 lerp3 = {std::lerp(mid2.x,end.x,t),std::lerp(mid2.y,end.y,t)};

    const Vector2 lerp4 = {std::lerp(lerp1.x,lerp2.x,t),std::lerp(lerp1.y,lerp2.y,t)};
    const Vector2 lerp5 = {std::lerp(lerp2.x,lerp3.x,t),std::lerp(lerp2.y,lerp3.y,t)};

    return {std::lerp(lerp4.x,lerp5.x,t),std::lerp(lerp4.y,lerp5.y,t)};
}

void DrawLineBezier(const Vector2 start, const Vector2 mid1, const Vector2 mid2, const Vector2 end, const float thickness, const Color color) {
    Vector2 previous = start;
    Vector2 current = { 0 };

    constexpr int SEGMENTS = 24;

    std::vector <Vector2> points;
    points.reserve(2*SEGMENTS + 2);
    for (int i = 0; i < 2*SEGMENTS + 2; i++) {
        points.push_back({0,0});
    }

    for (int i = 1; i <= 24; i++)
    {
        // Quadric curv lerping
        const float t = static_cast<float>(i) / SEGMENTS;
        current = cubicLerp(start, mid1, mid2, end, t);

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thickness/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    DrawTriangleStrip(points.data(), 2*SEGMENTS + 2, color);
}
