#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#define RAYGUI_IMPLEMENTATION
#include "../libs/raygui/src/raygui.h"

#include "main.hpp"
#include <string>
#include <filesystem>
#include <iostream>

#include "raymath.h"
#include "State.h"
#include "Transition.h"

using namespace std;

// Window setup
bool windowShouldClose = false;

constexpr char BLANK_CHAR = 31;

enum Mode {
    PAN,
    NEW_STATE,
    NEW_TRANSITION,
    MOVE_STATE,
    MOVE_TRANSITION,
    EDIT_TRANSITION,
    DELETE_STATE,
    DELETE_TRANSITION
};



// Loading resources
string assetPathPrefix = "../assets/";

vector <State> states = {{200,200,1},{400,300,2}};
vector <Transition> transitions = {
    {0,1,{{300,200}}},
    {1,0,{{350,500},{70,350}}}
};

float scale = 1;
Vector2 offset = {0,0};
Mode currentMode = PAN;
int addTransitionPart = 0;
TransitionCreateInfo newTransitionInfo;
char inputText[500]={0};
Color guiBackground = { 200, 200, 225, 255 };
int movingThingIndex =-1;
int moveThingSubIndex = -1;

void printMode() {
    switch (currentMode) {
        case PAN:
            cout << "PAN" << endl;
            break;
        case NEW_STATE:
            cout << "NEW_STATE" << endl;
            break;
        case NEW_TRANSITION:
            cout << "NEW_TRANSITION" << endl;
            break;
        case MOVE_STATE:
            cout << "MOVE_STATE" << endl;
            break;
        case MOVE_TRANSITION:
            cout << "MOVE_TRANSITION" << endl;
            break;
        case EDIT_TRANSITION:
            cout << "EDIT_TRANSITION" << endl;
            break;
        case DELETE_STATE:
            cout << "DELETE_STATE" << endl;
            break;
        case DELETE_TRANSITION:
            cout << "DELETE_TRANSITION" << endl;
            break;
    }
}

void init_app() {
    if (filesystem::exists("../assets") == false) {
        assetPathPrefix = "assets/";
    }
    
    // Load textures here
    
    // InitAudioDevice();
    // Load sounds/music here
    GuiSetIconScale(2);
    GuiSetStyle(DEFAULT,TEXT_SIZE,25);
}

bool app_loop() {
    float relDt = GetFrameTime() * 60.0f; // Calculate delta time in relation to 60 frames per second
    
    BeginDrawing();
    ClearBackground({255, 228, 209,255});

    for (Transition &t : transitions) {
        if (((currentMode == EDIT_TRANSITION || currentMode == MOVE_TRANSITION)&& movingThingIndex == -1) || currentMode == DELETE_TRANSITION) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            if (t.mouseOverText(mouse,states)) {
                t.draw(scale,states,offset,true);
            } else {
                t.draw(scale,states,offset,false);
            }
        } else {
            t.draw(scale,states,offset,false);
        }
    }

    for (State &state : states) {
        if ((currentMode == MOVE_STATE && movingThingIndex == -1) || currentMode == DELETE_STATE) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);

            if (state.mouseOver(mouse.x,mouse.y)) {
                state.draw(scale,offset,true);
            } else {
                state.draw(scale,offset,false);
            }
        } else {
            state.draw(scale,offset,false);
        }
    }

    if (currentMode == NEW_TRANSITION) {
        if (addTransitionPart > 0) {
            if (addTransitionPart == 1) {
                Vector2 startPos = states[newTransitionInfo.startIndex].getPosition();
                startPos = Vector2Add(startPos,offset);
                startPos = Vector2Scale(startPos,scale);
                Vector2 mouse = GetMousePosition();
                DrawLineV(startPos,mouse,BLACK);
            } else if (addTransitionPart == 2) {
                Vector2 startPos = states[newTransitionInfo.startIndex].getPosition();
                startPos = Vector2Add(startPos,offset);
                startPos = Vector2Scale(startPos,scale);
                Vector2 mouse = GetMousePosition();
                vector<Vector2> points;
                for (Vector2 point: newTransitionInfo.points) {
                    Vector2 p1 = Vector2Add(point,offset);
                    p1 = Vector2Scale(p1,scale);
                    points.push_back(p1);
                }
                if (points.size() <2) {
                    DrawLineBezier(startPos,points[0],mouse,2,BLACK);
                } else {
                    DrawLineBezier(startPos,points[0],points[1],mouse,2,BLACK);
                }
            } else if (addTransitionPart == 3) {
                DrawRectangle(100,100,1080,520,guiBackground);
                string infotext = "Enter read condition";
                DrawTextCentered(infotext,640,200,50,BLACK);

                GuiTextBox({440,300,400,50},inputText,100,true);
                if (IsKeyPressed(KEY_ENTER) || GuiButton({780,500,100,50},"Next")) {//if enter is pressed or the next button is pressed
                    //validate the input i guess
                    newTransitionInfo.match = inputText;
                    if (!newTransitionInfo.match.empty()) {
                        inputText[0] = '\0';
                        addTransitionPart = 4;
                    }
                } else if (GuiButton({400,500,100,50},"Blank")) {
                    int endInd = 0;
                    for (endInd = 0;endInd<100;endInd++) {
                        if (inputText[endInd] == '\0') {
                            break;
                        }
                    }
                    inputText[endInd] = BLANK_CHAR;//use this for the blank char, figure our custom rendering later
                    inputText[endInd+1] = '\0';
                }
            } else if (addTransitionPart == 4) {
                DrawRectangle(100,100,1080,520,guiBackground);
                string infotext = "Enter char to wright";
                DrawTextCentered(infotext,640,200,50,BLACK);
                GuiTextBox({615,300,50,50},inputText,2,true);
                if ((IsKeyPressed(KEY_ENTER) && !IsKeyPressedRepeat(KEY_ENTER)) || GuiButton({780,500,100,50},"Next")) {
                    if (inputText[0] != '\0') {
                        newTransitionInfo.wright = inputText[0];
                        inputText[0] = '\0';
                        addTransitionPart = 5;
                    }
                } else if (GuiButton({400,500,100,50},"Blank")) {
                    inputText[0] = BLANK_CHAR;
                    inputText[1] = '\0';
                }
            } else if (addTransitionPart == 5) {
                DrawRectangle(100,100,1080,520,guiBackground);
                string infotext = "Select head move direction";
                DrawTextCentered(infotext,640,200,50,BLACK);
                if (GuiButton({515,350,50,50},"L")) {
                    newTransitionInfo.move = 'L';
                    addTransitionPart = 0;
                    transitions.emplace_back(newTransitionInfo);
                }
                if (GuiButton({715,350,50,50},"R")) {
                    newTransitionInfo.move = 'R';
                    addTransitionPart = 0;
                    transitions.emplace_back(newTransitionInfo);
                }
            }
        }
    } else if (currentMode == MOVE_STATE) {
        if (movingThingIndex != -1) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            states[movingThingIndex].updatePosition(static_cast<int>(mouse.x),static_cast<int>(mouse.y));
        }
    } else if (currentMode == MOVE_TRANSITION) {
        if (movingThingIndex != -1) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            //draw the points
            for (Vector2 midPoint: transitions[movingThingIndex].getMidPoints()) {
                bool mouseOver = Vector2Distance(mouse,midPoint) < 15;
                midPoint = Vector2Add(midPoint,offset);
                midPoint = Vector2Scale(midPoint,scale);
                Color color = mouseOver ? Color{40,0,210,128} : Color{40,210,0,128};
                DrawCircleV(midPoint,15*scale,color);
            }

            if (moveThingSubIndex != -1) {
                transitions[movingThingIndex].setMidPoint(mouse,moveThingSubIndex);
            }
        }
    } else if (currentMode == EDIT_TRANSITION) {
        if (addTransitionPart == 3) {
            DrawRectangle(100,100,1080,520,guiBackground);
            string infotext = "Enter read condition";
            DrawTextCentered(infotext,640,200,50,BLACK);

            GuiTextBox({440,300,400,50},inputText,100,true);
            if (IsKeyPressed(KEY_ENTER) || GuiButton({780,500,100,50},"Next")) {//if enter is pressed or the next button is pressed
                //validate the input i guess
                newTransitionInfo.match = inputText;
                if (!newTransitionInfo.match.empty()) {
                    inputText[0] = newTransitionInfo.wright;
                    inputText[1] = '\0';
                    addTransitionPart = 4;
                }
            } else if (GuiButton({400,500,100,50},"Blank")) {
                int endInd = 0;
                for (endInd = 0;endInd<100;endInd++) {
                    if (inputText[endInd] == '\0') {
                        break;
                    }
                }
                inputText[endInd] = BLANK_CHAR;//use this for the blank char, figure our custom rendering later
                inputText[endInd+1] = '\0';
            }
        } else if (addTransitionPart == 4) {
            DrawRectangle(100,100,1080,520,guiBackground);
            string infotext = "Enter char to wright";
            DrawTextCentered(infotext,640,200,50,BLACK);
            GuiTextBox({615,300,50,50},inputText,2,true);
            if ((IsKeyPressed(KEY_ENTER) && !IsKeyPressedRepeat(KEY_ENTER)) || GuiButton({780,500,100,50},"Next")) {
                if (inputText[0] != '\0') {
                    newTransitionInfo.wright = inputText[0];
                    inputText[0] = '\0';
                    addTransitionPart = 5;
                }
            } else if (GuiButton({400,500,100,50},"Blank")) {
                inputText[0] = BLANK_CHAR;
                inputText[1] = '\0';
            }
        } else if (addTransitionPart == 5) {
            DrawRectangle(100,100,1080,520,guiBackground);
            string infotext = "Select head move direction";
            DrawTextCentered(infotext,640,200,50,BLACK);
            if (GuiButton({515,350,50,50},"L")) {
                newTransitionInfo.move = 'L';
                addTransitionPart = 0;
                transitions[movingThingIndex].setTransitionRules(newTransitionInfo.match,newTransitionInfo.wright,newTransitionInfo.move);
                movingThingIndex = -1;
            }
            if (GuiButton({715,350,50,50},"R")) {
                newTransitionInfo.move = 'R';
                addTransitionPart = 0;
                transitions[movingThingIndex].setTransitionRules(newTransitionInfo.match,newTransitionInfo.wright,newTransitionInfo.move);
                movingThingIndex = -1;
            }
        }
    }

    EndDrawing();

    //zoom scrolling
    float mouseChange = GetMouseWheelMove();

    if (mouseChange > 0.01 || mouseChange < -0.01) {
        float oldScale = scale;
        scale += mouseChange * 0.1f;
        scale = max(scale,0.1f);
        float scaleDif = scale / oldScale ;
        Vector2 mouse = GetMousePosition();
        offset.x = offset.x + mouse.x * (1/scale - 1/oldScale);
        offset.y = offset.y + mouse.y * (1/scale - 1/oldScale);
    }

    //mode switching
    if ((IsMouseButtonPressed(MOUSE_BUTTON_EXTRA) || IsKeyPressed(KEY_RIGHT_BRACKET)) && addTransitionPart == 0) {
        switch (currentMode) {
            case PAN://done
                currentMode = NEW_STATE;
                break;
            case NEW_STATE://done
                currentMode = NEW_TRANSITION;
                break;
            case NEW_TRANSITION://done
                currentMode = MOVE_STATE;
                break;
            case MOVE_STATE:
                currentMode = MOVE_TRANSITION;
                break;
            case MOVE_TRANSITION:
                currentMode = EDIT_TRANSITION;
                break;
            case EDIT_TRANSITION:
                currentMode = DELETE_STATE;
                break;
            case DELETE_STATE:
                currentMode = DELETE_TRANSITION;
                break;
            case DELETE_TRANSITION:
                currentMode = PAN;
                break;
        }
        printMode();
        movingThingIndex = -1;
        moveThingSubIndex = -1;
    } else if ((IsMouseButtonPressed(MOUSE_BUTTON_SIDE) || IsKeyPressed(KEY_LEFT_BRACKET)) && addTransitionPart == 0) {
        switch (currentMode) {
            case PAN:
                currentMode = DELETE_TRANSITION;
                break;
            case NEW_STATE:
                currentMode = PAN;
                break;
            case NEW_TRANSITION:
                currentMode = NEW_STATE;
                break;
            case MOVE_STATE:
                currentMode = NEW_TRANSITION;
                break;
            case MOVE_TRANSITION:
                currentMode = MOVE_STATE;
                break;
            case EDIT_TRANSITION:
                currentMode = MOVE_TRANSITION;
                break;
            case DELETE_STATE:
                currentMode = EDIT_TRANSITION;
                break;
            case DELETE_TRANSITION:
                currentMode = DELETE_STATE;
                break;
        }
        printMode();
        movingThingIndex = -1;
        moveThingSubIndex = -1;
    }

    if (currentMode == PAN) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, 1/scale);
            offset = Vector2Add(offset,delta);
        }
    } else if (currentMode == NEW_STATE) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            states.emplace_back(static_cast<int>(mouse.x),static_cast<int>(mouse.y),static_cast<int>(states.size()+1));

        }
    } else if (currentMode == NEW_TRANSITION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            if (addTransitionPart == 0) {
                //select the first state
                for (size_t i = 0; i < states.size(); i++) {
                    if (states[i].mouseOver(mouse.x,mouse.y)) {
                        newTransitionInfo = {};
                        newTransitionInfo.startIndex = static_cast<int>(i);
                        addTransitionPart = 1;
                        break;
                    }
                }
            } else if (addTransitionPart == 1) {
                //set the first mid point
                newTransitionInfo.points.push_back(mouse);
                addTransitionPart = 2;
            } else if (addTransitionPart == 2) {
                bool skip = false;
                //end state or second point
                for (size_t i = 0; i < states.size(); i++) {
                    if (states[i].mouseOver(mouse.x,mouse.y)) {
                        newTransitionInfo.endIndex = static_cast<int>(i);
                        addTransitionPart = 3;
                        skip = true;
                        inputText[0] = '\0';
                        break;
                    }
                }
                if (!skip && newTransitionInfo.points.size() <2) {
                    newTransitionInfo.points.push_back(mouse);
                }
            }
        }
    } else if (currentMode == MOVE_STATE) {
        Vector2 mouse = GetMousePosition();
        mouse = Vector2Scale(mouse,1/scale);
        mouse = Vector2Subtract(mouse,offset);
        if (movingThingIndex == -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < states.size(); i++) {
                if (states[i].mouseOver(mouse.x,mouse.y)) {
                    movingThingIndex = static_cast<int>(i);
                    break;
                }
            }
        } else if (movingThingIndex != -1 && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            movingThingIndex = -1;
        }
    } else if (currentMode == MOVE_TRANSITION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && movingThingIndex == -1) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            for (size_t i = 0; i < transitions.size(); i++) {
                if (transitions[i].mouseOverText(mouse,states)) {
                    movingThingIndex = static_cast<int>(i);
                    TextCopy(inputText,newTransitionInfo.match.c_str());
                    break;
                }
            }
        } else if (movingThingIndex != -1) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                Vector2 mouse = GetMousePosition();
                mouse = Vector2Scale(mouse,1/scale);
                mouse = Vector2Subtract(mouse,offset);

                for (size_t i = 0; i < transitions[movingThingIndex].getMidPoints().size(); i++) {
                    if (Vector2Distance(mouse,transitions[movingThingIndex].getMidPoints()[i]) < 15) {
                        moveThingSubIndex = static_cast<int>(i);
                        break;
                    }
                }
                if (moveThingSubIndex == -1) {
                    movingThingIndex = -1;
                }
            } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                moveThingSubIndex = -1;
            }
        }
    } else if (currentMode == EDIT_TRANSITION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && movingThingIndex == -1) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            for (size_t i = 0; i < transitions.size(); i++) {
                if (transitions[i].mouseOverText(mouse,states)) {
                    movingThingIndex = static_cast<int>(i);
                    newTransitionInfo.match = transitions[i].getMatchRule();
                    newTransitionInfo.wright = transitions[i].getWright();
                    newTransitionInfo.move = transitions[i].getMove();
                    addTransitionPart = 3;
                    TextCopy(inputText,newTransitionInfo.match.c_str());
                    break;
                }
            }
        }
    } else if (currentMode == DELETE_STATE) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            bool found = false;
            for (size_t i = 0; i < states.size(); i++) {
                if (found) {
                    states[i].decreaseId();
                } else {
                    if (states[i].mouseOver(mouse.x,mouse.y)) {
                        for (size_t j = 0; j < transitions.size(); j++) {
                            if (transitions[j].getStartIndex()==i || transitions[j].getEndIndex()==i) {
                                transitions.erase(transitions.begin()+static_cast<long long>(j));
                                j--;
                                continue;
                            }
                            if (transitions[j].getEndIndex()>i) {
                                transitions[j].decreaseEndIndex();
                            }
                            if (transitions[j].getStartIndex()>i) {
                                transitions[j].decreaseStartIndex();
                            }
                        }
                        states.erase(states.begin()+static_cast<long long>(i));
                        i--;
                        found = true;
                    }
                }
            }
        }
    } else if (currentMode == DELETE_TRANSITION) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            for (size_t i = 0; i < transitions.size(); i++) {
                if (transitions[i].mouseOverText(mouse,states)) {
                    transitions.erase(transitions.begin()+static_cast<long long>(i));
                    i--;
                }
            }
        }
    }


    return !windowShouldClose;
}

void deinit_app() {
    // Unload assets here

    // CloseAudioDevice();
}