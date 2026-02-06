#include "Transition.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include "helper.hpp"
#include "raymath.h"

Vector2 getRotationCoord(const double angle, const float scale) {
    const float x = scale * static_cast<float>(std::cos(angle));
    const float y = scale * static_cast<float>(std::sin(angle));
    return {x,y};
}

/**
 *
 * @param point The base point to work from
 * @param angle the angel to rotate the pint(in radians, this uses the standard lib)
 * @param scale How far from the base point the new point should be
 * @return A new point with a given rotation ans scale from the initial point
 */
Vector2 grotPoint(Vector2 point, const double angle, const float scale) {
    const float x = scale * static_cast<float>(std::cos(angle));
    const float y = scale * static_cast<float>(std::sin(angle));
    return {point.x+x,point.y+y};
}

std::string Transition::getMatchRule() {
    return match;
}

char Transition::getMove() const {
    return move;
}

char Transition::getWright() const {
    return wright;
}

void Transition::draw(float scale, std::vector<State> &states, Vector2 offset,bool highlight) {
    Color color = highlight ? YELLOW : BLACK;

    Vector2 startC = states[fromState].getPosition();
    Vector2 endC = states[toState].getPosition();
    startC.x += offset.x;
    startC.y += offset.y;
    endC.x += offset.x;
    endC.y += offset.y;
    startC.x *= scale;
    startC.y *= scale;
    endC.x *= scale;
    endC.y *= scale;


    std::vector<Vector2> midPointsL;
    for (Vector2 point : midPoints) {
        Vector2 point2 = point;
        point2.x += offset.x;
        point2.y += offset.y;
        point2.x *= scale;
        point2.y *= scale;
        midPointsL.push_back(point2);
    }

    Vector2 end;
    double startAngle = atan2(startC.y - midPointsL[0].y, startC.x - midPointsL[0].x);
    double endAngle;
    Vector2 rotIn = getRotationCoord(startAngle, scale*30);
    Vector2 start = {startC.x + rotIn.x, startC.y+rotIn.y};
    Vector2 moidPoint;
    if (midPoints.size() == 1) {
        endAngle = atan2(midPointsL[0].y - endC.y , midPointsL[0].x - endC.x);

        Vector2 rotOut = getRotationCoord(endAngle, scale*30);
        end = {endC.x + rotOut.x, endC.y+rotOut.y};
        DrawLineBezier(start,midPointsL[0],end,3*scale,color);
        moidPoint = quadritcLerp(start,midPointsL[0],end,0.5f);
    } else {
        endAngle = atan2(midPointsL[1].y - endC.y , midPointsL[1].x - endC.x);
        Vector2 rotOut = getRotationCoord(endAngle, scale*30);
        end = {endC.x + rotOut.x, endC.y+rotOut.y};
        DrawLineBezier(start,midPointsL[0],midPointsL[1],end,3*scale,color);
        moidPoint = cubicLerp(start,midPointsL[0],midPointsL[1],end,0.5f);
    }
    //draw the arrow at the end
    Vector2 e1 = grotPoint(end,endAngle+PI/6,scale*15);
    Vector2 e2 = grotPoint(end,endAngle-PI/6,scale*15);
    DrawTriangle(end,e1,e2,color);
    //draw text above the line
    std::string display = match+",";
    display += wright;
    display += ",";
    display += move;
    DrawTextCentered(display, static_cast<int>(moidPoint.x),static_cast<int>(moidPoint.y - 25*scale),static_cast<int>(scale * 20),BLUE);
}

void Transition::setTransitionRules(std::string &match, char wright, char move) {
    this->match = match;
    this->wright = wright;
    this->move = move;
}

bool Transition::mouseOverText(Vector2 mouse,std::vector<State> &states) {
    Vector2 startC = states[fromState].getPosition();
    Vector2 endC = states[toState].getPosition();
    Vector2 end;
    double startAngle = atan2(startC.y - midPoints[0].y, startC.x - midPoints[0].x);
    double endAngle;
    Vector2 rotIn = getRotationCoord(startAngle, 30);
    Vector2 start = {startC.x + rotIn.x, startC.y+rotIn.y};
    Vector2 moidPoint;
    if (midPoints.size() == 1) {
        endAngle = atan2(midPoints[0].y - endC.y , midPoints[0].x - endC.x);

        Vector2 rotOut = getRotationCoord(endAngle, 30);
        end = {endC.x + rotOut.x, endC.y+rotOut.y};
        moidPoint = quadritcLerp(start,midPoints[0],end,0.5f);
    } else {
        endAngle = atan2(midPoints[1].y - endC.y , midPoints[1].x - endC.x);
        Vector2 rotOut = getRotationCoord(endAngle, 30);
        end = {endC.x + rotOut.x, endC.y+rotOut.y};
        moidPoint = cubicLerp(start,midPoints[0],midPoints[1],end,0.5f);
    }
    std::string display = match+",";
    display += wright;
    display += ",";
    display += move;
    const int textLength = MeasureText(display.c_str(),20);
    const Rectangle box = {moidPoint.x-static_cast<float>(textLength)/2,moidPoint.y - 25,static_cast<float>(textLength),30};

    return mouse.x >= box.x && mouse.x <= box.x + box.width && mouse.y >= box.y && mouse.y <= box.y + box.height;
}

void HaltTransition::draw(float scale, std::vector<State> &states, Vector2 offset, bool highlight) {
    Color color = highlight ? YELLOW : BLACK;

    Vector2 startC = states[fromState].getPosition();
    startC.x += offset.x;
    startC.y += offset.y;
    startC.x *= scale;
    startC.y *= scale;

    std::vector<Vector2> midPointsL;
    for (Vector2 point : midPoints) {
        Vector2 point2 = point;
        point2.x += offset.x;
        point2.y += offset.y;
        point2.x *= scale;
        point2.y *= scale;
        midPointsL.push_back(point2);
    }

    double startAngle = atan2(startC.y - midPointsL[0].y, startC.x - midPointsL[0].x);
    Vector2 rotIn = getRotationCoord(startAngle, scale*30);
    Vector2 start = {startC.x + rotIn.x, startC.y+rotIn.y};
    Vector2 lep = grotPoint(midPointsL[0],startAngle,scale*5);

    DrawLineEx(start,lep,scale*3,color);
    Vector2 e1 = grotPoint(midPointsL[0],startAngle+PI/6,scale*15);
    Vector2 e2 = grotPoint(midPointsL[0],startAngle-PI/6,scale*15);
    DrawTriangle(midPointsL[0],e1,e2,color);

    Vector2 midPoint  ={std::lerp(start.x,midPointsL[0].x,0.6f),std::lerp(start.y,midPointsL[0].y,0.6f)};
    std::string display = match+",";
    display += wright;
    display += ",";
    display += move;
    DrawTextCentered(display, static_cast<int>(midPoint.x),static_cast<int>(midPoint.y - 25*scale),static_cast<int>(scale * 20),BLUE);
    Vector2 haltPoint = grotPoint(midPointsL[0],startAngle+PI,scale*30);
    std::string haltText = "Halt";
    DrawTextCentered(haltText,static_cast<int>(haltPoint.x),static_cast<int>(haltPoint.y-10*scale),static_cast<int>(25 * scale),BLACK);


}

bool HaltTransition::mouseOverText(Vector2 mouse, std::vector<State> &states) {
    Vector2 startC = states[fromState].getPosition();

    double startAngle = atan2(startC.y - midPoints[0].y, startC.x - midPoints[0].x);
    Vector2 rotIn = getRotationCoord(startAngle, 30);
    Vector2 start = {startC.x + rotIn.x, startC.y+rotIn.y};

    Vector2 midPoint = Vector2Add(start,midPoints[0]);
    midPoint = Vector2Scale(midPoint,0.5);

    std::string display = match+",";
    display += wright;
    display += ",";
    display += move;
    const int textLength = MeasureText(display.c_str(),20);
    const Rectangle box = {midPoint.x-static_cast<float>(textLength)/2,midPoint.y - 25,static_cast<float>(textLength),30};

    return mouse.x >= box.x && mouse.x <= box.x + box.width && mouse.y >= box.y && mouse.y <= box.y + box.height;
}
