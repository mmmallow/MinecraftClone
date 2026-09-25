#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <string>
#include "World.h"
#include "common.hpp"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "MinecraftClone");

    DisableCursor();

    Player player({5.0f, 90.0f, 5.0f});
    World world(player);
    player.world = &world;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        world.Update();

        BeginDrawing();
            world.Draw();
            DrawRectangle((GetScreenWidth() / 2.0f) - 2, (GetScreenHeight()/2.0f)-8, 4, 16, RAYWHITE);
            DrawRectangle((GetScreenWidth() / 2.0f) - 8, (GetScreenHeight()/2.0f)-2, 16, 4, RAYWHITE);
            DrawFPS(10, 10);
            DrawText(TextFormat("Pos: %.2f, %.2f, %.2f", player.position.x, player.position.y, player.position.z), 10, 30, 20, BLACK);
            DrawText(TextFormat("%s", GetBlockName(player.current_block_held).c_str()), 10, (float)GetScreenHeight() - 30, 20, BLACK);
        EndDrawing();
    }

    world.Destroy();

    CloseWindow();

    return 0;
}
