#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "World.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "MinecraftClone");

    DisableCursor();

    Player player({0.0f, 2.0f, 0.0f});

    World world(player);

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        world.Update();

        BeginDrawing();
            world.Draw();
            DrawRectangle((GetScreenWidth() / 2.0f)-2, (GetScreenHeight()/2.0f)-8, 4, 16, RAYWHITE);
            DrawRectangle((GetScreenWidth() / 2.0f) - 8, (GetScreenHeight()/2.0f)-2, 16, 4, RAYWHITE);
            DrawFPS(10, 10);
        EndDrawing();
    }

    world.Destroy();

    CloseWindow();

    return 0;
}
