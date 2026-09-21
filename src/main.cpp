#include <raylib.h>
#include <iostream>
#include <vector>
#include "Player.h"

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "MinecraftClone");

    DisableCursor();

    Player player({0.0f, 1.0f, 0.0f});


    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cubeModel = LoadModelFromMesh(cubeMesh);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BEIGE;

    std::vector<Vector3> blocks;
    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            blocks.push_back({i,-1.0f,j});
        }
    }
    blocks.push_back({3.0f, 0.0f, 3.0f});
    blocks.push_back({4.0f, 1.0f, 3.0f});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        player.Update(GetFrameTime());

        BeginDrawing();
            ClearBackground(SKYBLUE);
            BeginMode3D(player.camera);
                for (auto block : blocks) {
                    DrawModel(cubeModel, block, 1.0f, BEIGE);
                    DrawCubeWires(block, 1.0f, 1.0f, 1.0f, BLACK);
                }
            EndMode3D();

            DrawRectangle((GetScreenWidth() / 2.0f)-2, (GetScreenHeight()/2.0f)-8, 4, 16, RAYWHITE);
            DrawRectangle((GetScreenWidth() / 2.0f) - 8, (GetScreenHeight()/2.0f)-2, 16, 4, RAYWHITE);
        EndDrawing();
    }

    UnloadModel(cubeModel);

    CloseWindow();

    return 0;
}
