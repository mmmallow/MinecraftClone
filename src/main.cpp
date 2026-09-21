#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include "Player.h"

const Vector3 CUBE_SIZE = { 1.0f, 1.0f, 1.0f };

void check_collision (Player& player, Vector3 cube_pos) {
    BoundingBox player_box = {(Vector3){ player.position.x - player.size.x/2,
                                 player.position.y - player.size.y/2,
                                 player.position.z - player.size.z/2 },
                      (Vector3){ player.position.x + player.size.x/2,
                                 player.position.y + player.size.y/2,
                                 player.position.z + player.size.z/2 }};
    BoundingBox cube_box = {(Vector3){ cube_pos.x - CUBE_SIZE.x/2,
                                 cube_pos.y - CUBE_SIZE.y/2,
                                 cube_pos.z - CUBE_SIZE.z/2 },
                      (Vector3){ cube_pos.x + CUBE_SIZE.x/2,
                                 cube_pos.y + CUBE_SIZE.y/2,
                                 cube_pos.z + CUBE_SIZE.z/2 }};


    if (!CheckCollisionBoxes(player_box, cube_box)) return;

    Vector3 push_neg = Vector3Subtract(player_box.max, cube_box.min); // move player -axis
    Vector3 push_pos = Vector3Subtract(cube_box.max, player_box.min); // move player +axis

    Vector3 depth = { fminf(push_neg.x, push_pos.x),
                      fminf(push_neg.y, push_pos.y),
                      fminf(push_neg.z, push_pos.z) };
    Vector3 sign = { (push_neg.x < push_pos.x) ? -1.0f : 1.0f,
                     (push_neg.y < push_pos.y) ? -1.0f : 1.0f,
                     (push_neg.z < push_pos.z) ? -1.0f : 1.0f };

    if (depth.x <= depth.y && depth.x <= depth.z) {
        player.position.x += sign.x*depth.x;
    }
    else if (depth.y <= depth.x && depth.y <= depth.z) {
        player.position.y += sign.y*depth.y;
        player.velocity.y = 0.0f;
        if (sign.y > 0.0f) player.is_grounded = true; // landed on top of the block
    }
    else {
        player.position.z += sign.z*depth.z;
    }
}

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "MinecraftClone");

    DisableCursor();

    Player player({0.0f, 2.0f, 0.0f});


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

    blocks.push_back({-2.0f, 0.0f, -2.0f});
    blocks.push_back({-2.0f, 1.0f, -2.0f});
    blocks.push_back({-4.0f, 0.0f, -2.0f});
    blocks.push_back({-4.0f, 1.0f, -2.0f});
    blocks.push_back({-3.0f, 2.0f, -2.0f});

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        player.Update(GetFrameTime());

        for (auto it = blocks.begin(); it != blocks.end(); it++) {
            check_collision(player, *it);
        }

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
