#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#define RAYGUI_IMPLEMENTATION
#define NLOHMANN_JSON_DISABLE_CONSTEXPR
#include "../libs/raygui/src/raygui.h"

#include "main.hpp"
#include <string>
#include <filesystem>
#include <iostream>
#include <memory>
#include <fstream>

#include "raymath.h"
#include "State.h"
#include "Transition.h"
#include "Font.h"
#include "tinyfiledialogs.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

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
    DELETE_TRANSITION,
    SET_START_STATE,
    NEW_HALT_TRANSITION
};



// Loading resources
string assetPathPrefix = "../assets/";

vector <State> states = {{200,200,1},{400,300,2}};
vector <unique_ptr<Transition>> transitions = {};

float scale = 1;
Vector2 offset = {0,0};
Mode currentMode = PAN;
int addTransitionPart = 0;
TransitionCreateInfo newTransitionInfo;
char inputText[500]={0};
Color guiBackground = { 200, 200, 225, 255 };
int movingThingIndex =-1;
int moveThingSubIndex = -1;
bool eatMousePress = false;
int startState = -1;

bool saveImage = false;
bool dropDownOpen = false;


Font customFont;

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
        case SET_START_STATE:
            cout << "SET_START_STATE" << endl;
        case NEW_HALT_TRANSITION:
            cout << "NEW_HALT_TRANSITION" << endl;
    }
}

void drawTransitionIcon(int x,int y) {
    DrawLineBezier({static_cast<float>(x - 15),static_cast<float>(y - 15)},{static_cast<float>(x - 15),static_cast<float>(y + 15)},{static_cast<float>(x + 10),static_cast<float>(y + 15)},3,BLACK);
    //IMPORTANT! Triangle vertices MUST be specified counter clock wise!
    DrawTriangle({static_cast<float>(x + 10),static_cast<float>(y + 18)},{static_cast<float>(x+20),static_cast<float>(y + 15)},{static_cast<float>(x + 10),static_cast<float>(y + 10)},BLACK);
}

void init_app() {
    if (filesystem::exists("../assets") == false) {
        assetPathPrefix = "assets/";
    }
    
    // Load textures here
    customFont = createModifiedFont();

    // InitAudioDevice();
    // Load sounds/music here
    GuiSetIconScale(2);
    GuiSetStyle(DEFAULT,TEXT_SIZE,25);
    GuiSetFont(customFont);

    SetTargetFPS(120);
}

bool app_loop() {
    float relDt = GetFrameTime() * 60.0f; // Calculate delta time in relation to 60 frames per second

    bool buttonClicked = false;
    
    BeginDrawing();
    ClearBackground({255, 228, 209,255});


    for (unique_ptr<Transition> &t : transitions) {
        if (((currentMode == EDIT_TRANSITION || currentMode == MOVE_TRANSITION)&& movingThingIndex == -1) || currentMode == DELETE_TRANSITION) {
            Vector2 mouse = GetMousePosition();
            mouse = Vector2Scale(mouse,1/scale);
            mouse = Vector2Subtract(mouse,offset);
            if (t->mouseOverText(mouse,states)) {
                t->draw(scale,states,offset,true);
            } else {
                t->draw(scale,states,offset,false);
            }
        } else {
            t->draw(scale,states,offset,false);
        }
    }

    for (State &state : states) {
        if ((currentMode == MOVE_STATE && movingThingIndex == -1) || currentMode == DELETE_STATE || currentMode == SET_START_STATE || currentMode == NEW_HALT_TRANSITION) {
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

    if (startState != -1) {
        Vector2 ssp = states[startState].getPosition();
        ssp.x-=30;
        ssp = Vector2Add(ssp,offset);
        ssp = Vector2Scale(ssp,scale);
        DrawTriangle(ssp,{ssp.x-10*scale,ssp.y-10*scale},{ssp.x-10*scale,ssp.y+10*scale},RED);
        DrawRectangleRec({ssp.x-50*scale,ssp.y-3*scale,40*scale,6*scale},RED);
    }

    //mode switching UI
    if (currentMode == PAN) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({10,10,40,40},"#67#")) {
        //pan mode
        buttonClicked = true;
        currentMode = PAN;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }

    if (currentMode == NEW_STATE) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({55,10,40,40},"")) {
        //new state
        buttonClicked = true;
        currentMode = NEW_STATE;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    DrawCircle(75,30,17,WHITE);
    DrawRing({75,30},16,18,0,360,50,BLACK);
    DrawText("+",64,12,40,BLACK);

    if (currentMode == NEW_TRANSITION) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({100,10,40,40},"")) {
        //new transition
        buttonClicked = true;
        currentMode = NEW_TRANSITION;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    drawTransitionIcon(120,30);
    DrawText("+",109,12,40,{50,50,50,255});

    if (currentMode == MOVE_STATE) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({145,10,40,40},"")) {
        //move state
        buttonClicked = true;
        currentMode = MOVE_STATE;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    DrawCircle(165,30,17,WHITE);
    DrawRing({165,30},16,18,0,360,50,BLACK);
    GuiDrawIcon(68,148,13,2,BLACK);//move icon

    if (currentMode == MOVE_TRANSITION) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({190,10,40,40},"")) {
        //move transition
        buttonClicked = true;
        currentMode = MOVE_TRANSITION;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    drawTransitionIcon(210,30);
    GuiDrawIcon(68,193,13,2,{80,80,80,255});//move icon

    if (currentMode == EDIT_TRANSITION) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({235,10,40,40},"")) {
        //Edit transition
        buttonClicked = true;
        currentMode = EDIT_TRANSITION;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    drawTransitionIcon(255,30);
    GuiDrawIcon(23,238,13,2,{80,80,80,255});//move icon

    if (currentMode == DELETE_STATE) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({280,10,40,40},"")) {
        //Delete State
        buttonClicked = true;
        currentMode = DELETE_STATE;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    DrawCircle(300,30,17,WHITE);
    DrawRing({300,30},16,18,0,360,50,BLACK);
    GuiDrawIcon(143,285,13,2,BLACK);

    if (currentMode == DELETE_TRANSITION) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({325,10,40,40},"")) {
        //Delete transition
        buttonClicked = true;
        currentMode = DELETE_TRANSITION;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    drawTransitionIcon(345,30);
    GuiDrawIcon(143,330,13,2,{80,80,80,255});
    if (currentMode == SET_START_STATE) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({370,10,40,40},"")) {
        //Delete transition
        buttonClicked = true;
        currentMode = SET_START_STATE;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    DrawTriangle({400,30},{390,20},{390,40},RED);
    DrawRectangleRec({375,27,16,6},RED);

    if (currentMode == NEW_HALT_TRANSITION) GuiSetState(STATE_FOCUSED); else GuiSetState(STATE_NORMAL);
    if (GuiButton({415,10,40,40},"")) {
        //Delete transition
        buttonClicked = true;
        currentMode = NEW_HALT_TRANSITION;
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }
    DrawTriangle({428,30},{418,20},{418,40},BLACK);
    string tmp = "Ha";
    DrawTextCentered(tmp,441,20,20,BLACK);

    //end of mode switch ui

    if (buttonClicked) {
        printMode();
        EndDrawing();
        return true;
    }

    if (eatMousePress) {
        eatMousePress = false;
        return true;
    }
    //mode specific opperation ui
    if (currentMode == NEW_TRANSITION || currentMode == NEW_HALT_TRANSITION) {
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
                string infotext = "Enter char to write";
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
                    if (currentMode == NEW_HALT_TRANSITION) {
                        transitions.emplace_back(make_unique<HaltTransition>(newTransitionInfo));
                    } else {
                        transitions.emplace_back(make_unique<Transition>(newTransitionInfo));
                    }
                }
                if (GuiButton({715,350,50,50},"R")) {
                    newTransitionInfo.move = 'R';
                    addTransitionPart = 0;
                    if (currentMode == NEW_HALT_TRANSITION) {
                        transitions.emplace_back(make_unique<HaltTransition>(newTransitionInfo));
                    } else {
                        transitions.emplace_back(make_unique<Transition>(newTransitionInfo));
                    }
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
            for (Vector2 midPoint: transitions[movingThingIndex]->getMidPoints()) {
                bool mouseOver = Vector2Distance(mouse,midPoint) < 15;
                midPoint = Vector2Add(midPoint,offset);
                midPoint = Vector2Scale(midPoint,scale);
                Color color = mouseOver ? Color{40,0,210,128} : Color{40,210,0,128};
                DrawCircleV(midPoint,15*scale,color);
            }

            if (moveThingSubIndex != -1) {
                transitions[movingThingIndex]->setMidPoint(mouse,moveThingSubIndex);
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
            string infotext = "Enter char to write";
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
                transitions[movingThingIndex]->setTransitionRules(newTransitionInfo.match,newTransitionInfo.wright,newTransitionInfo.move);
                movingThingIndex = -1;
            }
            if (GuiButton({715,350,50,50},"R")) {
                newTransitionInfo.move = 'R';
                addTransitionPart = 0;
                transitions[movingThingIndex]->setTransitionRules(newTransitionInfo.match,newTransitionInfo.wright,newTransitionInfo.move);
                movingThingIndex = -1;
            }
        }
    }
    int a =-1;
    //the options are specified in test sperated by a semi colon, edit mode is if it is open, I think a is the clicked option
    if (GuiDropdownBox({1070,10,200,30},"New Machine;#12#Export Image;#6#Load;#2#Save",&a,dropDownOpen)) {
        if (dropDownOpen) {
            /* 0 = new
             * 1 = export
             * 2 = load
             * 3 = save
             */
            if (a==0) {
                if (tinyfd_messageBox("Are You Sure","Delete current machine and make a new one?","yesno","question",0)) {
                    startState = -1;
                    transitions.erase(transitions.begin(),transitions.end());
                    states.erase(states.begin(),states.end());
                    movingThingIndex =-1;
                    moveThingSubIndex = -1;
                    addTransitionPart = 0;
                    offset = {0,0};
                    scale = 1.0f;
                }
            } else if (a==1) {
                saveImage = true;
            } else if (a==2) {
                string thinkingDifferent = "*.thinkDifferent";//so apparently on macos if you just put json as the accepted type it can not figure out what a json file is, but if you put any other file type first, then it works. Thanks for requiring us to think different apple
                string jsonFt = "*.json";
                const char ** fileTypes = static_cast<const char **>(malloc(sizeof(char *)*2));
                fileTypes[0] = thinkingDifferent.c_str();
                fileTypes[1] = jsonFt.c_str();
                const char * fileToLoad = tinyfd_openFileDialog("Load Machine", "",2,fileTypes,"JSON file",false);
                if (fileToLoad != nullptr) {
                    string fileName = fileToLoad;
                    if (!fileName.ends_with(".json")) {
                        fileName = fileName.append(".json");
                    }
                    loadMachine(fileName);
                }

                free(fileTypes);
            } else if (a==3) {
                string jsonFt = "*.json";
                const char ** fileTypes = static_cast<const char **>(malloc(sizeof(char *)));
                fileTypes[0] = jsonFt.c_str();
                const char * fileToLoad = tinyfd_saveFileDialog("Save Machine", "",1,fileTypes,"JSON file");
                if (fileToLoad != nullptr) {
                    string fileName = fileToLoad;
                    if (!fileName.ends_with(".json")) {
                        fileName = fileName.append(".json");
                    }
                    saveMachine(fileName);
                }
                free(fileTypes);
            }
        }
        dropDownOpen = !dropDownOpen;
    }

    EndDrawing();

    //zoom scrolling
    float mouseChange = GetMouseWheelMove();

    if (mouseChange > 0.01 || mouseChange < -0.01) {
        float oldScale = scale;
        scale += mouseChange * 0.1f;
        scale = max(scale,0.1f);
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
                currentMode = SET_START_STATE;
                break;
            case SET_START_STATE:
                currentMode = NEW_HALT_TRANSITION;
                break;
            case NEW_HALT_TRANSITION:
                currentMode = PAN;
                break;
        }
        printMode();
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    } else if ((IsMouseButtonPressed(MOUSE_BUTTON_SIDE) || IsKeyPressed(KEY_LEFT_BRACKET)) && addTransitionPart == 0) {
        switch (currentMode) {
            case PAN:
                currentMode = NEW_HALT_TRANSITION;
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
            case SET_START_STATE:
                currentMode = DELETE_TRANSITION;
                break;
            case NEW_HALT_TRANSITION:
                currentMode = SET_START_STATE;
                break;
        }
        printMode();
        movingThingIndex = -1;
        moveThingSubIndex = -1;
        addTransitionPart = 0;
    }

    //dont process further if the mouse is in the button area
    Vector2 areaMouse = GetMousePosition();
    if (areaMouse.x >= 10 && areaMouse.x <=455 && areaMouse.y >= 10 && areaMouse.y <=50) {
        return true;
    }
    //dont process futher over the drop down box
    //{1070,10,200,30}
    float maxDropPx = 40.0f + 4.0f * 30.0f * static_cast<float>(dropDownOpen);
    if (areaMouse.x >= 1070 && areaMouse.x <= 1270 && areaMouse.y >= 10 && areaMouse.y <= maxDropPx) {
        return true;
    }

    if (currentMode != PAN) {
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, 1/scale);
            offset = Vector2Add(offset,delta);
        }
    }

    if (currentMode == PAN) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)){
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
    } else if (currentMode == NEW_TRANSITION || currentMode == NEW_HALT_TRANSITION) {
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
                if (currentMode == NEW_HALT_TRANSITION) {
                    addTransitionPart = 3;
                } else {
                    addTransitionPart = 2;
                }
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
                    eatMousePress = true;
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
                if (transitions[i]->mouseOverText(mouse,states)) {
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

                for (size_t i = 0; i < transitions[movingThingIndex]->getMidPoints().size(); i++) {
                    if (Vector2Distance(mouse,transitions[movingThingIndex]->getMidPoints()[i]) < 15) {
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
                if (transitions[i]->mouseOverText(mouse,states)) {
                    movingThingIndex = static_cast<int>(i);
                    newTransitionInfo.match = transitions[i]->getMatchRule();
                    newTransitionInfo.wright = transitions[i]->getWright();
                    newTransitionInfo.move = transitions[i]->getMove();
                    addTransitionPart = 3;
                    TextCopy(inputText,newTransitionInfo.match.c_str());
                    eatMousePress = true;
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
                            if (transitions[j]->getStartIndex()==i || transitions[j]->getEndIndex()==i) {
                                transitions.erase(transitions.begin()+static_cast<long long>(j));
                                j--;
                                continue;
                            }
                            if (transitions[j]->getEndIndex()>i) {
                                transitions[j]->decreaseEndIndex();
                            }
                            if (transitions[j]->getStartIndex()>i) {
                                transitions[j]->decreaseStartIndex();
                            }
                        }
                        states.erase(states.begin()+static_cast<long long>(i));
                        if (startState == i) {
                            startState = -1;
                        } else if (startState >= static_cast<int>(i)) {
                            startState --;

                        }
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
                if (transitions[i]->mouseOverText(mouse,states)) {
                    transitions.erase(transitions.begin()+static_cast<long long>(i));
                    i--;
                }
            }
        }
    } else if (currentMode == SET_START_STATE) {
        Vector2 mouse = GetMousePosition();
        mouse = Vector2Scale(mouse,1/scale);
        mouse = Vector2Subtract(mouse,offset);
        if (movingThingIndex == -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < states.size(); i++) {
                if (states[i].mouseOver(mouse.x,mouse.y)) {
                    startState = static_cast<int>(i);
                    break;
                }
            }
        }
    }

    //handle movement keys
    if (addTransitionPart < 3) {
        constexpr float keyPanSpeed = 6.0f;
        if (IsKeyDown(KEY_LEFT)) {
            offset.x += keyPanSpeed * 1/scale * relDt;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            offset.x -= keyPanSpeed * 1/scale * relDt;
        }
        if (IsKeyDown(KEY_UP)) {
            offset.y += keyPanSpeed * 1/scale * relDt;
        }
        if (IsKeyDown(KEY_DOWN)) {
            offset.y -= keyPanSpeed * 1/scale * relDt;
        }
    }

    if (saveImage) {
        string png = "*.png";
        const char ** fileTypes = static_cast<const char **>(malloc(sizeof(char *)));
        fileTypes[0] = png.c_str();
        const char * selectedFile_c = tinyfd_saveFileDialog("Save Image","",1,fileTypes,nullptr);
        if (selectedFile_c != nullptr) {
            string selectedFile = selectedFile_c;
            if (!selectedFile.empty()) {
                if (!selectedFile.ends_with(".png")) {
                    selectedFile.append(".png");
                }
                cout << selectedFile << endl;
                exportToRenderedImage(selectedFile);
            }

        }
        free(fileTypes);
        saveImage = false;
    }


    return !windowShouldClose;
}

void exportToRenderedImage(const std::string &fileName) {
    //step 1 find the bounding rectangle of all the drawable elements (add like 20 - 50 on each side for some padding)
    Vector2 minPoint = {static_cast<float>(0xFFFFFFF),static_cast<float>(0xFFFFFFF)};
    Vector2 maxPoint = {};

    for (State state: states) {
        Vector2 pos = state.getPosition();
        maxPoint = Vector2Max(maxPoint,pos);
        minPoint = Vector2Min(minPoint,pos);
    }

    for (const unique_ptr<Transition> &transition: transitions) {
        Vector2 pos = transition->getTextPoint(states);
        if (transition->isHalt()) {
            pos = transition->getMidPoints()[0];
        }
        maxPoint = Vector2Max(maxPoint,pos);
        minPoint = Vector2Min(minPoint,pos);

    }

    //step 2 create a camera offset to place the stuf in the correct place, this is just the min point with the padding
    Vector2 camOffset = Vector2Add(minPoint,{-70,-70});
    camOffset = Vector2Scale(camOffset,-1);

    Vector2 imageSize = Vector2Subtract(maxPoint,minPoint);
    imageSize = Vector2Add(imageSize,{140,140});

    //step 3 render all the things to the texture
    RenderTexture2D render_texture = LoadRenderTexture(static_cast<int>(imageSize.x),static_cast<int>(imageSize.y));
    BeginTextureMode(render_texture);

    ClearBackground(WHITE);
    for (unique_ptr<Transition> &t : transitions) {
        t->draw(1,states,camOffset,false);
    }

    for (State &state : states) {
        state.draw(1,camOffset,false);
    }

    if (startState != -1) {
        Vector2 ssp = states[startState].getPosition();
        ssp.x-=30;
        ssp = Vector2Add(ssp,camOffset);
        DrawTriangle(ssp,{ssp.x-10,ssp.y-10},{ssp.x-10,ssp.y+10},RED);
        DrawRectangleRec({ssp.x-50,ssp.y-3,40,6},RED);
    }

    EndTextureMode();


    //step 4 save the texture to an image
    Image imageToSave = LoadImageFromTexture(render_texture.texture);
    //step 4.5 flip the image so it is not upside down
    ImageFlipVertical(&imageToSave);

    bool success = ExportImage(imageToSave,fileName.c_str());
    if (!success) {
        cerr << "Failed to save image." << endl;
    }

    UnloadImage(imageToSave);
    UnloadRenderTexture(render_texture);
}

void loadMachine(const string &fileName) {
    //step 1 load the json file
    json file = parseJsonNe(fileName);

    //step 2 clear the current machine
    startState = -1;
    transitions.erase(transitions.begin(),transitions.end());
    states.erase(states.begin(),states.end());
    movingThingIndex =-1;
    moveThingSubIndex = -1;
    addTransitionPart = 0;
    offset = {0,0};
    scale = 1.0f;

    //step 3 load the states
    json jsonStates = file["states"];
    for (const auto& state : jsonStates) {
        states.emplace_back(state,static_cast<int>(states.size())+1);
    }

    //step 4 load the transitions
    json jsonTransitions = file["transitions"];
    for (auto transition : jsonTransitions) {
        string type = transition["type"];
        if (type == "halt") {
            transitions.emplace_back(make_unique<HaltTransition>(transition));
        } else {
            transitions.emplace_back(make_unique<Transition>(transition));
        }
    }
    //step 5 read the start state
    startState = file["startState"];
}

void saveMachine(const std::string& fileName) {
    json save{};
    save["states"] = {};
    for (State& state : states) {
        save["states"].push_back(state.asJson());
    }
    save["transitions"] = {};
    for (unique_ptr<Transition> &transition : transitions) {
        save["transitions"].push_back(transition->asJson());
    }
    save["startState"] = startState;
    ofstream outFile(fileName);
    outFile << save;
    outFile.close();
}

void deinit_app() {
    // Unload assets here
    UnloadCustomFont(customFont);
    // CloseAudioDevice();
}

Font getModifiedFont() {
    return customFont;
}