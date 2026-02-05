#include "../libs/raylib/src/raylib.h"
#include "helper.hpp"
#include "main.hpp"

#include <filesystem>



void web_loop()
{
    static int old_w = 0, old_h = 0;

    int w = getWindowWidth();
    int h = getWindowHeight();
    if (w != old_w || h != old_h)
    {
        SetWindowSize(w, h);
    }
    app_loop();
}

int main()
{
    // Setup window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TuringDesigner");
    // Load icon here
    // Image windowIcon;
    // if (std::filesystem::exists("../assets") == false) {
    //     windowIcon = LoadImage("assets/icon.png");
    // } else {
    //     windowIcon = LoadImage("../assets/icon.png");
    // }
    // SetWindowIcon(windowIcon);

    // SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    init_app();


    while (app_loop() && !WindowShouldClose()){}
    deinit_app();

    // Cleanup
    CloseWindow();
    return 0;
}
